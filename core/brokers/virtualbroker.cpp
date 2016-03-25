/*
 * virtualbroker.cpp
 */

#include "virtualbroker.h"
#include "log.h"

namespace sp = std::placeholders;

VirtualBroker::VirtualBroker(double startCash, const QuoteTable::Ptr& table) : m_cash(startCash),
	m_table(table)
{
	m_table->setTickCallback(std::bind(&VirtualBroker::incomingTick, this, sp::_1, sp::_2));
}

VirtualBroker::~VirtualBroker()
{
}

void VirtualBroker::save(std::ostream& stream)
{
	stream << "SUR;" << m_cash << ";" << std::endl;
	for(const auto& position : m_portfolio)
	{
		stream << position.first << ";" << position.second << ";" << std::endl;
	}
}

void VirtualBroker::load(std::istream& stream)
{
	std::string line;
	while(std::getline(stream, line))
	{
		auto firstSemicolon = line.find(';');
		if(firstSemicolon == std::string::npos)
			continue;
		auto ticker = line.substr(0, firstSemicolon);
		auto position = line.substr(firstSemicolon + 1, line.size() - firstSemicolon - 2);

		if(ticker == "SUR")
		{
			m_cash = std::atof(position.c_str());
		}
		else
		{
			m_portfolio[ticker] = std::atoi(position.c_str());
		}
	}
}

void VirtualBroker::submitOrder(const Order::Ptr& order)
{
	boost::unique_lock<boost::recursive_mutex> lock(m_mutex);
	m_allOrders.push_back(order);
	LOG(INFO) << "VirtualBroker: submitted order: " << order->stringRepresentation();

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
		LOG(INFO) << "Limit order";
		auto bidTick = m_table->lastQuote(order->security(), goldmine::Datatype::BestBid);
		auto offerTick = m_table->lastQuote(order->security(), goldmine::Datatype::BestOffer);
		auto bid = bidTick.value.toDouble();
		auto offer = offerTick.value.toDouble();

		LOG(INFO) << bid << "/" << offer;

		if(order->operation() == Order::Operation::Buy)
		{
			if((offerTick.value <= order->price()) && (offer != 0))
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
			else
			{
				addPendingOrder(order);
			}
		}
		else if(order->operation() == Order::Operation::Sell)
		{
			if((bid != 0) && (bidTick.value >= order->price()))
			{
				LOG(INFO) << "Order OK";
				executeSellAt(order, bidTick.value, bidTick.timestamp, bidTick.useconds);
			}
			else
			{
				addPendingOrder(order);
			}
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
	boost::unique_lock<boost::recursive_mutex> lock(m_mutex);
	m_orderCallbacks.push_back(callback);
}

void VirtualBroker::unregisterOrderCallback(const OrderCallback& callback)
{
}

void VirtualBroker::registerTradeCallback(const TradeCallback& callback)
{
	boost::unique_lock<boost::recursive_mutex> lock(m_mutex);
	m_tradeCallbacks.push_back(callback);
}

Order::Ptr VirtualBroker::order(int localId)
{
	auto it = std::find_if(m_pendingOrders.begin(), m_pendingOrders.end(), [&](const Order::Ptr& order) { return order->localId() == localId; } );
	if(it == m_pendingOrders.end())
	{
		it = std::find_if(m_allOrders.begin(), m_allOrders.end(), [&](const Order::Ptr& order) { return order->localId() == localId; } );
		if(it == m_allOrders.end())
			return Order::Ptr();
	}

	return *it;
}

std::list<std::string> VirtualBroker::accounts()
{
	std::list<std::string> result;
	result.push_back("demo");
	return result;
}

void VirtualBroker::addPendingOrder(const Order::Ptr& order)
{
	order->updateState(Order::State::Submitted);
	m_pendingOrders.push_back(order);
	m_table->enableTicker(order->security());
}

void VirtualBroker::unsubscribeFromTickerIfNeeded(const std::string& ticker)
{
	auto it = std::find_if(m_pendingOrders.begin(), m_pendingOrders.end(), [&](const Order::Ptr& order) { return order->security() == ticker; } );
	if(it == m_pendingOrders.end())
	{
		m_table->disableTicker(ticker);
	}
}

void VirtualBroker::incomingTick(const std::string& ticker, const goldmine::Tick& tick)
{
	boost::unique_lock<boost::recursive_mutex> lock(m_mutex);
	LOG(DEBUG) << "VirtualBroker::incomingTick: " << ticker;
	auto it = m_pendingOrders.begin();
	while(it != m_pendingOrders.end())
	{
		auto next = it;
		++next;

		auto order = *it;
		LOG(DEBUG) << "Order: " << order->stringRepresentation();
		if(next != m_pendingOrders.end())
		{
			LOG(DEBUG) << "Next Order: " << (*next)->stringRepresentation();
		}
		if(order->security() == ticker)
		{
			if((order->operation() == Order::Operation::Buy) && (tick.value <= order->price()) &&
					((tick.datatype == (int)goldmine::Datatype::BestOffer) || (tick.datatype == (int)goldmine::Datatype::Price)))
			{
				executeBuyAt(order, order->price(), tick.timestamp, tick.useconds);
				m_pendingOrders.erase(it);
				unsubscribeFromTickerIfNeeded(order->security());
				orderStateUpdated(order);
			}
			else if((order->operation() == Order::Operation::Sell) && (tick.value >= order->price()) &&
					((tick.datatype == (int)goldmine::Datatype::BestBid) || (tick.datatype == (int)goldmine::Datatype::Price)))
			{
				executeSellAt(order, order->price(), tick.timestamp, tick.useconds);
				m_pendingOrders.erase(it);
				unsubscribeFromTickerIfNeeded(order->security());
				orderStateUpdated(order);
			}
		}

		it = next;
	}
}

void VirtualBroker::orderStateUpdated(const Order::Ptr& order)
{
	LOG(DEBUG) << "VirtualBroker::orderStateUpdated: " << order->stringRepresentation();
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
	LOG(INFO) << "VB: executeBuyAt: " << price.toDouble();
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
	LOG(INFO) << "VB: executeSellAt: " << price.toDouble();
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

