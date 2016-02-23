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
	LOG(DEBUG) << "VirtualBroker: submitted order: " << order->stringRepresentation();

	m_pendingOrders.push_back(order);
	if(order->type() == Order::OrderType::Market)
	{
		auto bid = m_table->lastQuote(order->security(), goldmine::Datatype::BestBid).value.toDouble();
		auto offer = m_table->lastQuote(order->security(), goldmine::Datatype::BestOffer).value.toDouble();

		_TRACE << bid << "/" << offer;

		if(order->operation() == Order::Operation::Buy)
		{
			if(offer == 0)
			{
				_TRACE << "No offers";
				order->updateState(Order::State::Rejected);
			}
			else
			{
				double volume = offer * order->amount();
				if(m_cash < volume)
				{
					_TRACE << "Not enough cash";
					order->updateState(Order::State::Rejected);
				}
				else
				{
					_TRACE << "Order OK";
					order->updateState(Order::State::Executed);
					m_portfolio[order->security()] += order->amount();
					m_cash -= volume;
				}
			}
			orderStateUpdated(order);
		}
		else
		{
			if(bid == 0)
			{
				_TRACE << "No bids";
				order->updateState(Order::State::Rejected);
			}
			else
			{
				m_cash += bid * order->amount();
				m_portfolio[order->security()] -= order->amount();

				order->updateState(Order::State::Executed);
			}
			orderStateUpdated(order);
		}
	}
	else
	{
		LOG(WARNING) << "Requested unsupported order type";
	}
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

std::list<Position> VirtualBroker::positions()
{
	std::list<Position> result;

	for(auto it = m_portfolio.begin(); it != m_portfolio.end(); ++it)
	{
		result.push_back(Position {it->first, it->second} );
	}
	return result;
}
