
#include "fakebroker.h"

void FakeBroker::submitOrder(const Order::Ptr& order)
{
	m_orders.push_back(order);
}

void FakeBroker::cancelOrder(const Order::Ptr& order)
{
	m_orders.remove(order);
}

void FakeBroker::registerOrderCallback(const OrderCallback& callback)
{
	m_orderCallback = callback;
}
void FakeBroker::unregisterOrderCallback(const OrderCallback& callback)
{
}

void FakeBroker::registerTradeCallback(const TradeCallback& callback)
{
	m_tradeCallback = callback;
}

Order::Ptr FakeBroker::order(int localId)
{
	auto it = std::find_if(m_orders.begin(), m_orders.end(),
			[&](const Order::Ptr& order) { return order->localId() == localId; });
	if(it != m_orders.end())
		return *it;

	return Order::Ptr();
}

std::list<std::string> FakeBroker::accounts()
{
	return m_accounts;
}

std::list<Position> FakeBroker::positions()
{
	return m_positions;
}

