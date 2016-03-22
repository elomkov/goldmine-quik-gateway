
#include "catch.hpp"

#include "core/brokers/virtualbroker.h"

#include "goldmine/data.h"

static Order::Ptr gs_lastOrder;
static void orderCallback(const Order::Ptr order)
{
	gs_lastOrder = order;
}

static Trade gs_lastTrade;
static void tradeCallback(const Trade& trade)
{
	gs_lastTrade = trade;
}

TEST_CASE("VirtualBroker", "[core][broker]")
{
	auto quoteTable = std::make_shared<QuoteTable>();
	VirtualBroker broker(100000, quoteTable);
	broker.registerOrderCallback(orderCallback);
	broker.registerTradeCallback(tradeCallback);

	SECTION("Simple market buy")
	{
		goldmine::Tick tick;
		tick.datatype = (int)goldmine::Datatype::BestOffer;
		tick.timestamp = 0;
		tick.useconds = 0;
		tick.value = goldmine::decimal_fixed(100);
		tick.volume = 0;
		quoteTable->updateQuote("TEST", tick);

		broker.submitOrder(std::make_shared<Order>(1, "demo", "TEST", 0, 1, Order::Operation::Buy, Order::OrderType::Market));

		REQUIRE(gs_lastOrder != nullptr);
		REQUIRE(gs_lastOrder->state() == Order::State::Executed);

		REQUIRE(gs_lastTrade.price == tick.value);
		REQUIRE(gs_lastTrade.amount == 1);
		REQUIRE(gs_lastTrade.operation == Order::Operation::Buy);
		REQUIRE(gs_lastTrade.account == "demo");
		REQUIRE(gs_lastTrade.ticker == "TEST");
		REQUIRE(gs_lastTrade.timestamp == tick.timestamp);
		REQUIRE(gs_lastTrade.useconds == tick.useconds);
	}

	SECTION("Simple market sell")
	{
		goldmine::Tick tick;
		tick.datatype = (int)goldmine::Datatype::BestBid;
		tick.timestamp = 0;
		tick.useconds = 0;
		tick.value = goldmine::decimal_fixed(100);
		tick.volume = 0;
		quoteTable->updateQuote("TEST", tick);

		broker.submitOrder(std::make_shared<Order>(1, "demo", "TEST", 0, 1, Order::Operation::Sell, Order::OrderType::Market));

		REQUIRE(gs_lastOrder != nullptr);
		REQUIRE(gs_lastOrder->state() == Order::State::Executed);

		REQUIRE(gs_lastTrade.price == tick.value);
		REQUIRE(gs_lastTrade.amount == 1);
		REQUIRE(gs_lastTrade.operation == Order::Operation::Sell);
		REQUIRE(gs_lastTrade.account == "demo");
		REQUIRE(gs_lastTrade.ticker == "TEST");
		REQUIRE(gs_lastTrade.timestamp == tick.timestamp);
		REQUIRE(gs_lastTrade.useconds == tick.useconds);
	}

	SECTION("Market buy && no offer => rejected")
	{
		broker.submitOrder(std::make_shared<Order>(1, "demo", "TEST", 0, 1, Order::Operation::Buy, Order::OrderType::Market));

		REQUIRE(gs_lastOrder != nullptr);
		REQUIRE(gs_lastOrder->state() == Order::State::Rejected);
	}

	SECTION("Market sell && no bids => rejected")
	{
		broker.submitOrder(std::make_shared<Order>(1, "demo", "TEST", 0, 1, Order::Operation::Sell, Order::OrderType::Market));

		REQUIRE(gs_lastOrder != nullptr);
		REQUIRE(gs_lastOrder->state() == Order::State::Rejected);
	}

	SECTION("Simple limit buy - corresponding offer is already available")
	{
		goldmine::Tick tick;
		tick.datatype = (int)goldmine::Datatype::BestOffer;
		tick.timestamp = 41;
		tick.useconds = 5555;
		tick.value = goldmine::decimal_fixed(101);
		tick.volume = 0;
		quoteTable->updateQuote("TEST", tick);

		broker.submitOrder(std::make_shared<Order>(1, "demo", "TEST", 110, 1, Order::Operation::Buy, Order::OrderType::Limit));

		REQUIRE(gs_lastOrder != nullptr);
		REQUIRE(gs_lastOrder->state() == Order::State::Executed);

		// If offer was already available, order is filled at offer's price, 
		// even though order's price is higher
		REQUIRE(gs_lastTrade.price == tick.value);
		REQUIRE(gs_lastTrade.amount == 1);
		REQUIRE(gs_lastTrade.operation == Order::Operation::Buy);
		REQUIRE(gs_lastTrade.account == "demo");
		REQUIRE(gs_lastTrade.ticker == "TEST");
		REQUIRE(gs_lastTrade.timestamp == tick.timestamp);
		REQUIRE(gs_lastTrade.useconds == tick.useconds);
	}

	SECTION("Simple limit sell - corresponding bid is already available")
	{
		goldmine::Tick tick;
		tick.datatype = (int)goldmine::Datatype::BestBid;
		tick.timestamp = 41;
		tick.useconds = 5555;
		tick.value = goldmine::decimal_fixed(100);
		tick.volume = 0;
		quoteTable->updateQuote("TEST", tick);

		broker.submitOrder(std::make_shared<Order>(1, "demo", "TEST", 90, 1, Order::Operation::Sell, Order::OrderType::Limit));

		REQUIRE(gs_lastOrder != nullptr);
		REQUIRE(gs_lastOrder->state() == Order::State::Executed);

		REQUIRE(gs_lastTrade.price == tick.value);
		REQUIRE(gs_lastTrade.amount == 1);
		REQUIRE(gs_lastTrade.operation == Order::Operation::Sell);
		REQUIRE(gs_lastTrade.account == "demo");
		REQUIRE(gs_lastTrade.ticker == "TEST");
		REQUIRE(gs_lastTrade.timestamp == tick.timestamp);
		REQUIRE(gs_lastTrade.useconds == tick.useconds);
	}

	SECTION("Simple limit buy - incoming tick")
	{
		broker.submitOrder(std::make_shared<Order>(1, "demo", "TEST", 110, 1, Order::Operation::Buy, Order::OrderType::Limit));

		goldmine::Tick tick;
		tick.datatype = (int)goldmine::Datatype::BestOffer;
		tick.timestamp = 41;
		tick.useconds = 5555;
		tick.value = goldmine::decimal_fixed(100);
		tick.volume = 0;
		quoteTable->updateQuote("TEST", tick);

		REQUIRE(gs_lastOrder != nullptr);
		REQUIRE(gs_lastOrder->state() == Order::State::Executed);

		// Limit order executed at order price
		REQUIRE(gs_lastTrade.price == goldmine::decimal_fixed(110));
		REQUIRE(gs_lastTrade.amount == 1);
		REQUIRE(gs_lastTrade.operation == Order::Operation::Buy);
		REQUIRE(gs_lastTrade.account == "demo");
		REQUIRE(gs_lastTrade.ticker == "TEST");
		REQUIRE(gs_lastTrade.timestamp == tick.timestamp);
		REQUIRE(gs_lastTrade.useconds == tick.useconds);
	}

	SECTION("Simple limit sell - incoming tick")
	{
		broker.submitOrder(std::make_shared<Order>(1, "demo", "TEST", 90, 1, Order::Operation::Sell, Order::OrderType::Limit));

		goldmine::Tick tick;
		tick.datatype = (int)goldmine::Datatype::BestBid;
		tick.timestamp = 41;
		tick.useconds = 5555;
		tick.value = goldmine::decimal_fixed(100);
		tick.volume = 0;
		quoteTable->updateQuote("TEST", tick);

		REQUIRE(gs_lastOrder != nullptr);
		REQUIRE(gs_lastOrder->state() == Order::State::Executed);

		// Limit order executed at order price
		REQUIRE(gs_lastTrade.price == goldmine::decimal_fixed(90));
		REQUIRE(gs_lastTrade.amount == 1);
		REQUIRE(gs_lastTrade.operation == Order::Operation::Sell);
		REQUIRE(gs_lastTrade.account == "demo");
		REQUIRE(gs_lastTrade.ticker == "TEST");
		REQUIRE(gs_lastTrade.timestamp == tick.timestamp);
		REQUIRE(gs_lastTrade.useconds == tick.useconds);
	}
}

