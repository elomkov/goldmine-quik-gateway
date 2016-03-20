
#include "catch.hpp"

#include "core/brokers/virtualbroker.h"

#include "goldmine/data.h"

static Order::Ptr gs_lastOrder;
static void orderCallback(const Order::Ptr order)
{
	gs_lastOrder = order;
}

TEST_CASE("VirtualBroker", "[core][broker]")
{
	auto quoteTable = std::make_shared<QuoteTable>();
	VirtualBroker broker(100000, quoteTable);
	broker.registerOrderCallback(orderCallback);

	SECTION("Simple market buy")
	{
		goldmine::Tick tick;
		tick.datatype = (int)goldmine::Datatype::BestOffer;
		tick.value = goldmine::decimal_fixed(100);
		tick.volume = 0;
		quoteTable->updateQuote("TEST", tick);

		broker.submitOrder(std::make_shared<Order>(1, "demo", "TEST", 0, 1, Order::Operation::Buy, Order::OrderType::Market));

		REQUIRE(gs_lastOrder != nullptr);
		REQUIRE(gs_lastOrder->state() == Order::State::Executed);
	}
}

