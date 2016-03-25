
#include "core/brokerserver.h"

#include "goldmine/data.h"

#include "catch.hpp"
#include "json.h"

#include "mocks/fakebroker.h"

#include <zmq.hpp>

void sendOrder(const Json::Value& order, zmq::socket_t& socket)
{
	Json::Value root;
	root["order"] = order;

	Json::FastWriter writer;
	auto json = writer.write(root);

	zmq::message_t msgDelimiter;
	zmq::message_t msgJson(json.data(), json.size());

	socket.send(msgDelimiter, ZMQ_SNDMORE);
	socket.send(msgJson, 0);
}

TEST_CASE("BrokerServer", "[core][broker]")
{
	zmq::context_t context;
	BrokerServer server(context, "inproc://broker-control");
	auto broker = std::make_shared<FakeBroker>();
	server.addBroker(broker);
	server.start();

	zmq::socket_t client(context, ZMQ_DEALER);
	client.connect("inproc://broker-control");

	SECTION("Order submission")
	{
		broker->m_accounts.push_back("foo");
		Json::Value order;
		order["id"] = 1;
		order["account"] = "foo";
		order["security"] = "bar";
		order["type"] = "limit";
		order["amount"] = 2;
		order["price"] = 19.73;
		order["operation"] = "buy";

		sendOrder(order, client);

		while(broker->m_orders.empty()); // TODO timeout

		auto lastOrder = broker->m_orders.back();
		REQUIRE(lastOrder->clientAssignedId() == 1);
		REQUIRE(lastOrder->account() == "foo");
		REQUIRE(lastOrder->security() == "bar");
		REQUIRE(lastOrder->type() == Order::OrderType::Limit);
		REQUIRE(fabs(lastOrder->price() - 19.73) < 0.001);
		REQUIRE(lastOrder->operation() == Order::Operation::Buy);
	}

	server.stop();
}

