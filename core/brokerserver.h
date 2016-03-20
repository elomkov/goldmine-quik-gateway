/*
 * brokerserver.h
 */

#ifndef CORE_BROKERSERVER_H_
#define CORE_BROKERSERVER_H_

#include "zmq.hpp"
#include <boost/thread.hpp>
#include "json.h"
#include "broker.h"
#include "binary/byte_array.h"

class BrokerServer
{
public:
	BrokerServer(zmq::context_t& context, const std::string& controlEp);
	virtual ~BrokerServer();

	void start();
	void stop();

	void addBroker(const Broker::Ptr& broker);

private:
	void run();
	void handleControlSocket(zmq::socket_t& control);
	void handleCommand(const Json::Value& root, const byte_array& peerId);

	void handleSocketStateUpdate(zmq::socket_t& control, zmq::socket_t& stateSocket);
	void orderCallback(const Order::Ptr& order);

	Broker::Ptr findBrokerForAccount(const std::string& account);

private:
	bool m_running;
	boost::thread m_thread;
	zmq::context_t& m_context;
	zmq::socket_t m_control;
	std::string m_controlEp;
	std::vector<Broker::Ptr> m_brokers;
	std::map<int, byte_array> m_orderPeers;
	std::unique_ptr<zmq::socket_t> m_orderSocket;
};

#endif /* CORE_BROKERSERVER_H_ */
