/*
 * eventloop.cpp
 */

#include "eventloop.h"
#include "datasink.h"
#include "time/timesource.h"
#include "tables/currentparametertableparser.h"
#include "log.h"
#include <cstring>

EventLoop::EventLoop(zmq::context_t& ctx,
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

EventLoop::~EventLoop()
{
}

void EventLoop::start()
{
	m_thread = boost::thread(std::bind(&EventLoop::run, this));
}

void EventLoop::stop()
{
	m_run = false;
	m_thread.join();
}

void EventLoop::run()
{
	m_datasink->connect(m_incomingPipeEndpoint);
	auto timesource = std::make_shared<TimeSource>();
	auto tableparser = std::make_shared<CurrentParameterTableParser>("default", m_datasink, timesource);
	m_dataImportServer->registerTableParser(tableparser);

	m_control.bind(m_controlEndpoint);
	m_incomingPipe.bind(m_incomingPipeEndpoint);

	zmq_pollitem_t pollitems[] = { { (void*)m_control, 0, ZMQ_POLLIN, 0 },
		{ (void*)m_incomingPipe, 0, ZMQ_POLLIN, 0 } };

	m_run = true;
	while(m_run)
	{
		int rc = zmq::poll(pollitems, 2, 1000);
		if(rc < 0)
			throw std::runtime_error("zmq::poll error, returned " + std::to_string(rc));

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
}

void EventLoop::handleControlSocket()
{

}

void EventLoop::handleIncomingPipe()
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

void EventLoop::sendStreamPacket(const std::string& ticker, int datatype, void* packet, size_t packetSize)
{
	for(const auto& client : m_clients)
	{
		if(client->acceptsStream(ticker, (goldmine::Datatype)datatype))
		{
			sendPacketTo(client->peerId(), ticker, packet, packetSize);
		}
	}
}

void EventLoop::sendPacketTo(const byte_array& peerId, const std::string& ticker, void* packet, size_t packetSize)
{
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
