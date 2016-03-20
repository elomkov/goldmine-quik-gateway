
#include "quotesourceserver.h"

#include "datasink.h"
#include "time/timesource.h"
#include "tables/currentparametertableparser.h"
#include "tables/alldealstableparser.h"
#include "log.h"
#include <cstring>
#include "json.h"
#include "exceptions.h"

QuotesourceServer::QuotesourceServer(zmq::context_t& ctx,
		const std::string& controlEp,
		const std::string& incomingPipeEp) : m_run(false),
	m_control(ctx, ZMQ_ROUTER),
	m_controlEndpoint(controlEp),
	m_incomingPipe(ctx, ZMQ_PULL),
	m_incomingPipeEndpoint(incomingPipeEp),
	m_datasink(std::make_shared<DataSink>(ctx))
{
	m_dataImportServer = std::make_shared<DataImportServer>("gold", "default");
}

QuotesourceServer::~QuotesourceServer()
{
}

void QuotesourceServer::start()
{
	m_thread = boost::thread(std::bind(&QuotesourceServer::run, this));
	m_run = true;
}

void QuotesourceServer::stop()
{
	m_run = false;
	m_thread.join();
}

void QuotesourceServer::run()
{
	m_datasink->connect(m_incomingPipeEndpoint);
	auto timesource = std::make_shared<TimeSource>();

	m_control.bind(m_controlEndpoint);
	m_incomingPipe.bind(m_incomingPipeEndpoint);

	zmq_pollitem_t pollitems[] = { { (void*)m_control, 0, ZMQ_POLLIN, 0 },
		{ (void*)m_incomingPipe, 0, ZMQ_POLLIN, 0 } };

	while(m_run)
	{
		try
		{
			int rc = zmq::poll(pollitems, 2, 100);
			if(rc < 0)
				BOOST_THROW_EXCEPTION(ZmqError() << errinfo_str("zmq::poll error, returned " + std::to_string(rc)));

			if(rc > 0)
			{
				if(pollitems[0].revents & ZMQ_POLLIN)
				{
					handleControlSocket();
				}
				else if(pollitems[1].revents & ZMQ_POLLIN)
				{
					handleIncomingPipe();
				}
				else
				{
					LOG(WARNING) << "zmq::poll returned, but no input data is available";
				}
			}
		}
		catch(const GoldmineQuikGatewayException& e)
		{
			LOG(WARNING) << "Exception in Quotesource server: " << e.what();
		}
		catch(const std::runtime_error& e)
		{
			LOG(WARNING) << "Exception in Quotesource server: " << e.what();
		}
	}
}

void QuotesourceServer::handleControlSocket()
{
	zmq::message_t msgPeerId;
	zmq::message_t msgDelimiter;
	LOG(INFO) << "Incoming control message";

	if(!m_control.recv(&msgPeerId, ZMQ_NOBLOCK))
		BOOST_THROW_EXCEPTION(ProtocolError() << errinfo_str("Control: unable to read peer ID"));

	uint8_t* peerIdData = reinterpret_cast<uint8_t*>(msgPeerId.data());
	byte_array peerId(peerIdData, peerIdData + msgPeerId.size());

	if(!m_control.recv(&msgDelimiter, ZMQ_NOBLOCK))
		BOOST_THROW_EXCEPTION(ProtocolError() << errinfo_str("Control: unable to read delimiter"));

	zmq::message_t msgMessageType;
	if(!m_control.recv(&msgMessageType, ZMQ_NOBLOCK))
		BOOST_THROW_EXCEPTION(ProtocolError() << errinfo_str("Control: unable to read message type"));

	if(msgMessageType.size() == 0)
		throw std::runtime_error("Invalid message type frame");
	int packetType = ((uint8_t*)msgMessageType.data())[0];
	if(packetType == (int)goldmine::MessageType::Control)
	{
		zmq::message_t msgCommand;
		if(!m_control.recv(&msgCommand, ZMQ_NOBLOCK))
			BOOST_THROW_EXCEPTION(ProtocolError() << errinfo_str("Control: unable to read control message body"));

		handleControlCommand(peerId, reinterpret_cast<uint8_t*>(msgCommand.data()), msgCommand.size());
	}
	else if(packetType == (int)goldmine::MessageType::StreamCredit)
	{
		auto client = getClient(peerId);
		if(!client)
			BOOST_THROW_EXCEPTION(ProtocolError() << errinfo_str("Control: got credit from unsubscribed client"));

		int credits = 1;
		if(msgMessageType.size() > 1)
		{
			uint32_t* creditsPtr = (uint32_t*)((uint8_t*)msgMessageType.data() + 1);
			credits = *creditsPtr;
		}

		client->incrementCredits(credits);
	}
}

void QuotesourceServer::handleIncomingPipe()
{
	zmq::message_t msgTicker;
	zmq::message_t msgDatatype;
	zmq::message_t msgPacket;

	if(!m_incomingPipe.recv(&msgTicker, ZMQ_NOBLOCK))
		throw std::runtime_error("Unable to recv from incoming pipe: ticker");

	if(!m_incomingPipe.recv(&msgDatatype, ZMQ_NOBLOCK))
		throw std::runtime_error("Unable to recv from incoming pipe: datatype");

	if(!m_incomingPipe.recv(&msgPacket, ZMQ_NOBLOCK))
		throw std::runtime_error("Unable to recv from incoming pipe: packet");

	std::string ticker(reinterpret_cast<char*>(msgTicker.data()), msgTicker.size());
	int datatype = *(reinterpret_cast<uint32_t*>(msgDatatype.data()));

	sendStreamPacket(ticker, datatype, msgPacket.data(), msgPacket.size());
}

void QuotesourceServer::sendStreamPacket(const std::string& ticker, int datatype, void* packet, size_t packetSize)
{
	for(const auto& client : m_clients)
	{
		if(client->acceptsStream(ticker, (goldmine::Datatype)datatype) && (client->getCredits() > 0))
		{
			sendPacketTo(client->peerId(), ticker, packet, packetSize);
			client->decrementCredits(1);
		}
	}
}

void QuotesourceServer::sendPacketTo(const byte_array& peerId, const std::string& ticker, void* packet, size_t packetSize)
{
	LOG(DEBUG) << "Sending packet: " << ticker << "; " << packetSize;

	zmq::message_t msgPeerId(peerId.size());
	memcpy(msgPeerId.data(), peerId.data(), peerId.size());

	zmq::message_t msgDelimiter;

	zmq::message_t msgMessageType(1);
	*((uint8_t*)msgMessageType.data()) = (uint8_t)goldmine::MessageType::Stream;

	zmq::message_t msgTicker(ticker.size());
	memcpy(msgTicker.data(), ticker.c_str(), ticker.size());

	zmq::message_t msgPacket(packetSize);
	memcpy(msgPacket.data(), packet, packetSize);

	m_control.send(msgPeerId, ZMQ_SNDMORE);
	m_control.send(msgDelimiter, ZMQ_SNDMORE);
	m_control.send(msgMessageType, ZMQ_SNDMORE);
	m_control.send(msgTicker, ZMQ_SNDMORE);
	m_control.send(msgPacket, 0);
}

void QuotesourceServer::sendControlResponse(const byte_array& peerId)
{
	zmq::message_t msgPeerId(peerId.size());
	memcpy(msgPeerId.data(), peerId.data(), peerId.size());

	zmq::message_t msgDelimiter;

	zmq::message_t msgMessageType(1);
	*((uint8_t*)msgMessageType.data()) = (uint8_t)goldmine::MessageType::Control;

	std::string response = " { \"result\" : \"success\" } ";

	zmq::message_t msgResponse(response.size());
	memcpy(msgResponse.data(), response.c_str(), response.size());
	m_control.send(msgPeerId, ZMQ_SNDMORE);
	m_control.send(msgDelimiter, ZMQ_SNDMORE);
	m_control.send(msgMessageType, ZMQ_SNDMORE);
	m_control.send(msgResponse, 0);
}

Client::Ptr QuotesourceServer::getClient(const byte_array& peerId)
{
	for(const auto& client : m_clients)
	{
		if(client->peerId() == peerId)
			return client;
	}
	return Client::Ptr();
}

void QuotesourceServer::deleteClient(const byte_array& peerId)
{
	auto it = std::find_if(m_clients.begin(), m_clients.end(),
			[&](const Client::Ptr& client) { return client->peerId() == peerId; });
	if(it != m_clients.end())
		m_clients.erase(it);
}

void QuotesourceServer::handleControlCommand(const byte_array& peerId, uint8_t* buffer, size_t size)
{
	LOG(DEBUG) << "handleControlCommand";
	Json::Value root;
	Json::Reader reader;
	std::string err;

	if(!reader.parse((const char*)buffer, (const char*)buffer + size, root))
		throw std::runtime_error("Unable to parse incoming command: " + err);

	auto cmd = root["command"].asString();
	LOG(DEBUG) << "handleControlCommand: " << cmd;

	auto tickersValue = root["tickers"];
	std::vector<std::string> tickers;
	for(const auto& ticker : tickersValue)
	{
		tickers.push_back(ticker.asString());
	}
	for(const auto& ticker : tickers)
	{
		LOG(INFO) << "Incoming ticker: " << ticker;
	}

	if(cmd == "start")
	{
		if(tickers.size() == 0)
			throw std::runtime_error("Empty ticker list");
		auto client = getClient(peerId);
		if(client)
			deleteClient(peerId);
		client = std::make_shared<Client>(peerId);
		for(const auto ticker : tickers)
		{
			client->addStreamMatcher(ticker);
		}
		m_clients.push_back(client);
	}

	sendControlResponse(peerId);
	LOG(DEBUG) << "handleControlCommand done";
}

DataImportServer::Ptr QuotesourceServer::dataImportServer() const
{
	return m_dataImportServer;
}

DataSink::Ptr QuotesourceServer::datasink() const
{
	return m_datasink;
}
