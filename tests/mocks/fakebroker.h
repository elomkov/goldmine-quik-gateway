
#ifndef FAKEBROKER
#define FAKEBROKER

#include "core/broker.h"

class FakeBroker : public Broker
{
public:
	virtual void submitOrder(const Order::Ptr& order);
	virtual void cancelOrder(const Order::Ptr& order);

	virtual void registerOrderCallback(const OrderCallback& callback);
	virtual void unregisterOrderCallback(const OrderCallback& callback);

	virtual void registerTradeCallback(const TradeCallback& callback);

	virtual Order::Ptr order(int localId);

	virtual std::list<std::string> accounts();

	virtual std::list<Position> positions();

	std::list<Order::Ptr> m_orders;
	std::list<Position> m_positions;
	std::list<std::string> m_accounts;

	OrderCallback m_orderCallback;
	TradeCallback m_tradeCallback;
};

#endif /* ifndef FAKEBROKER */

