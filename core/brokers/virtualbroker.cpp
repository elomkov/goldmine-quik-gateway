/*
 * virtualbroker.cpp
 */

#include "virtualbroker.h"
#include "log.h"

VirtualBroker::VirtualBroker(double startCash, const QuoteTable::Ptr& table) : m_cash(startCash),
	m_table(table)
{
}

VirtualBroker::~VirtualBroker()
{
}

void VirtualBroker::submitOrder(const Order::Ptr& order)
{
	LOG(INFO) << "VirtualBroker: submitted order: " << order->stringRepresentation();

	m_pendingOrders.push_back(order);
	if(order->type() == Order::OrderType::Market)
	{
		auto bidTick = m_table->lastQuote(order->security(), goldmine::Datatype::BestBid); 
		auto offerTick = m_table->lastQuote(order->security(), goldmine::Datatype::BestOffer); 
		auto bid = bidTick.value.toDouble();
		auto offer = offerTick.value.toDouble();

		LOG(INFO) << bid << "/" << offer;

		if(order->operation() == Order::Operation::Buy)
		{
			if(offer == 0)
			{
				LOG(INFO) << "No offers";
				order->updateState(Order::State::Rejected);
			}
			else
			{
				double volume = offer * order->amount();
				if(m_cash < volume)
				{
					LOG(INFO) << "Not enough cash";
					order->updateState(Order::State::Rejected);
				}
				else
				{
					LOG(INFO) << "Order OK";
					executeBuyAt(order, offerTick.value, offerTick.timestamp, offerTick.useconds);
				}
			}
			orderStateUpdated(order);
		}
		else
		{
			if(bid == 0)
			{
				LOG(INFO) << "No bids";
				order->updateState(Order::State::Rejected);
			}
			else
			{
				executeSellAt(order, bidTick.value, bidTick.timestamp, bidTick.useconds);
			}
			orderStateUpdated(order);
		}
	}
	else if(order->type() == Order::OrderType::Limit)
	{
		auto bidTick = m_table->lastQuote(order->security(), goldmine::Datatype::BestBid); 
		auto offerTick = m_table->lastQuote(order->security(), goldmine::Datatype::BestOffer); 
		auto bid = bidTick.value.toDouble();
		auto offer = offerTick.value.toDouble();

		LOG(INFO) << bid << "/" << offer;

		if(order->operation() == Order::Operation::Buy)
		{
			if(offerTick.value < order->price())
			{
				double volume = offer * order->amount();
				if(m_cash < volume)
				{
					LOG(INFO) << "Not enough cash";
					order->updateState(Order::State::Rejected);
				}
				else
				{
					LOG(INFO) << "Order OK";
					executeBuyAt(order, offerTick.value, offerTick.timestamp, offerTick.useconds);
				}
			}
		}
		else if(order->operation() == Order::Operation::Sell)
		{
			executeSellAt(order, bidTick.value, bidTick.timestamp, bidTick.useconds);
		}
		orderStateUpdated(order);
	}
	else
	{
		LOG(WARNING) << "Requested unsupported order type";
	}
	LOG(INFO) << "Cash: " << m_cash;
}

void VirtualBroker::cancelOrder(const Order::Ptr& order)
{

}

void VirtualBroker::registerOrderCallback(const OrderCallback& callback)
{
	m_orderCallbacks.push_back(callback);
}

void VirtualBroker::unregisterOrderCallback(const OrderCallback& callback)
{
}

void VirtualBroker::registerTradeCallback(const TradeCallback& callback)
{
	m_tradeCallbacks.push_back(callback);
}

Order::Ptr VirtualBroker::order(int localId)
{
	auto it = std::find_if(m_pendingOrders.begin(), m_pendingOrders.end(), [&](const Order::Ptr& order) { return order->localId() == localId; } );
	if(it == m_pendingOrders.end())
		return Order::Ptr();

	return *it;
}

std::list<std::string> VirtualBroker::accounts()
{
	std::list<std::string> result;
	result.push_back("demo");
	return result;
}

void VirtualBroker::orderStateUpdated(const Order::Ptr& order)
{
	for(const auto& c : m_orderCallbacks)
	{
		c(order);
	}
}

void VirtualBroker::emitTrade(const Trade& trade)
{
	for(const auto& c : m_tradeCallbacks)
	{
		c(trade);
	}
}

void VirtualBroker::executeBuyAt(const Order::Ptr& order, const goldmine::decimal_fixed& price, uint64_t timestamp, uint32_t useconds)
{
	double volume = price.toDouble() * order->amount();
	order->updateState(Order::State::Executed);
	m_portfolio[order->security()] += order->amount();
	m_cash -= volume;

	Trade trade;
	trade.orderId = order->clientAssignedId();
	trade.account = order->account();
	trade.price = price;
	trade.amount = order->amount();
	trade.operation = order->operation();
	trade.ticker = order->security();
	trade.timestamp = timestamp;
	trade.useconds = useconds;
	emitTrade(trade);
}

void VirtualBroker::executeSellAt(const Order::Ptr& order, const goldmine::decimal_fixed& price, uint64_t timestamp, uint32_t useconds)
{
	m_cash += price.toDouble() * order->amount();
	m_portfolio[order->security()] -= order->amount();

	order->updateState(Order::State::Executed);

	Trade trade;
	trade.orderId = order->clientAssignedId();
	trade.account = order->account();
	trade.price = price;
	trade.amount = order->amount();
	trade.operation = order->operation();
	trade.ticker = order->security();
	trade.timestamp = timestamp;
	trade.useconds = useconds;
	emitTrade(trade);
}

std::list<Position> VirtualBroker::positions()
{
	std::list<Position> result;

	for(auto it = m_portfolio.begin(); it != m_portfolio.end(); ++it)
	{
		result.push_back(Position {it->first, it->second} );
	}
	return result;
}
