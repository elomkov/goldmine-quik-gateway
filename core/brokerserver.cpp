/*
 * brokerserver.cpp
 */

#include "brokerserver.h"
#include <functional>
#include "log.h"

static std::string serializeOrderState(Order::State state)
{
	switch(state)
	{
	case Order::State::Cancelled:
		return "cancelled";
	case Order::State::Executed:
		return "executed";
	case Order::State::PartiallyExecuted:
		return "partially-executed";
	case Order::State::Rejected:
		return "rejected";
	case Order::State::Submitted:
		return "submitted";
	case Order::State::Unsubmitted:
		return "unsubmitted";
	}
	return "unknown";
}

BrokerServer::BrokerServer(zmq::context_t& context, const std::string& controlEp, const Broker::Ptr& broker) :
	m_running(false),
	m_context(context),
	m_control(context, ZMQ_ROUTER),
	m_controlEp(controlEp),
	m_broker(broker)
{
	m_broker->registerOrderCallback(std::bind(&BrokerServer::orderCallback, this, std::placeholders::_1));
}

BrokerServer::~BrokerServer()
{
}

void BrokerServer::start()
{
	m_thread = boost::thread(std::bind(&BrokerServer::run, this));
}

void BrokerServer::stop()
{
	m_running = false;
	m_thread.join();
}

void BrokerServer::run()
{
	m_running = true;
	LOG(INFO) << "Broker server started";
	m_control.bind(m_controlEp.c_str());

	zmq::socket_t orderStateSocket(m_context, ZMQ_PULL);
	orderStateSocket.bind("inproc://order-state-socket");

	zmq_pollitem_t pollitems[] = { { (void*)m_control, 0, ZMQ_POLLIN, 0 },
			{ (void*)orderStateSocket, 0, ZMQ_POLLIN, 0 } };

	while(m_running)
	{
		int rc = zmq::poll(pollitems, 1, 1000);
		if(rc < 0)
			throw std::runtime_error("BrokerServer: zmq::poll error, returned " + std::to_string(rc));

		if(rc > 0)
		{
			if(pollitems[0].revents == ZMQ_POLLIN)
			{
				try
				{
					handleControlSocket(m_control);
				}
				catch(const std::runtime_error& e)
				{
					LOG(WARNING) << "Exception when handling message from control socket: " << e.what();
				}
			}
			else if(pollitems[1].revents == ZMQ_POLLIN)
			{
				handleSocketStateUpdate(m_control, orderStateSocket);
			}
		}
	}
}

void BrokerServer::handleControlSocket(zmq::socket_t& control)
{
	zmq::message_t msgPeerId;
	zmq::message_t msgDelimiter;
	LOG(INFO) << "BrokerServer: incoming message";

	if(!control.recv(&msgPeerId, ZMQ_NOBLOCK))
		throw std::runtime_error("Control: unable to read peer id");

	uint8_t* peerIdData = reinterpret_cast<uint8_t*>(msgPeerId.data());
	byte_array peerId(peerIdData, peerIdData + msgPeerId.size());

	if(!control.recv(&msgDelimiter, ZMQ_NOBLOCK))
		throw std::runtime_error("Control: unable to read delimiter");

	zmq::message_t msgCommand;
	if(!control.recv(&msgCommand, ZMQ_NOBLOCK))
		throw std::runtime_error("Control: unable to read control packet");

	Json::Value root;
	Json::Reader reader;
	std::string err;

	const char* buffer = (const char*)msgCommand.data();
	if(!reader.parse(buffer, buffer + msgCommand.size(), root))
		throw std::runtime_error("Control: unable to parse incoming json");

	std::string json(buffer, buffer + msgCommand.size());

	LOG(DEBUG) << "BrokerServer: incoming JSON: " << json;

	handleCommand(root, peerId);
}

void BrokerServer::handleCommand(const Json::Value& root, const byte_array& peerId)
{
	if(!root["order"].isNull())
	{
		auto order = root["order"];
		int id = order["id"].asInt();
		auto account = order["account"].asString();
		auto security = order["security"].asString();
		auto type = order["type"].asString();
		auto price = order["price"].asDouble();
		auto amount = order["amount"].asInt();
		auto operation = order["operation"].asString();

		Order::Operation op;
		if(operation == "buy")
			op = Order::Operation::Buy;
		else if(operation == "sell")
			op = Order::Operation::Sell;
		else
			throw std::runtime_error("Invalid operation specified: " + operation);

		Order::OrderType t;
		if(type == "limit")
			t = Order::OrderType::Limit;
		else if(type == "market")
			t = Order::OrderType::Market;
		else
			throw std::runtime_error("Invalid order type specified: " + type);

		auto brokerOrder = std::make_shared<Order>(id, account, security, price, amount, op, t);

		m_orderPeers[brokerOrder->localId()] = peerId;
		m_broker->submitOrder(brokerOrder);
	}
}

void BrokerServer::handleSocketStateUpdate(zmq::socket_t& control, zmq::socket_t& stateSocket)
{
	zmq::message_t msg;

	stateSocket.recv(&msg, ZMQ_NOBLOCK);

	int id = 0;
	memcpy((void*)&id, msg.data(), 4);

	auto it = m_orderPeers.find(id);
	if(it == m_orderPeers.end())
		throw std::runtime_error("Order state changed, but peer is unknown");

	auto peerId = it->second;

	auto order = m_broker->order(id);

	zmq::message_t msgPeerId(peerId.size());
	memcpy(msgPeerId.data(), peerId.data(), peerId.size());

	zmq::message_t msgDelimiter;

	Json::Value root;
	Json::Value orderValue;
	orderValue["id"] = order->clientAssignedId();
	orderValue["new-state"] = serializeOrderState(order->state());
	root["order"] = orderValue;

	Json::FastWriter writer;
	auto json = writer.write(root);

	zmq::message_t msgJson(json.size());
	memcpy(msgJson.data(), json.data(), json.size());

	control.send(msgPeerId, ZMQ_SNDMORE);
	control.send(msgDelimiter, ZMQ_SNDMORE);
	control.send(msgJson, 0);
}

void BrokerServer::orderCallback(const Order::Ptr& order)
{
	if(!m_orderSocket)
	{
		m_orderSocket = std::make_unique<zmq::socket_t>(m_context, ZMQ_PUSH);
		m_orderSocket->connect("inproc://order-state-socket");
	}

	int orderId = order->localId();
	zmq::message_t msg(4);
	memcpy(msg.data(), (void*)&orderId, 4);

	m_orderSocket->send(msg, 0);
}
