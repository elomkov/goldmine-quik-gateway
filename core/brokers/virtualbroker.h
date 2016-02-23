/*
 * virtualbroker.h
 */

#ifndef CORE_BROKERS_VIRTUALBROKER_H_
#define CORE_BROKERS_VIRTUALBROKER_H_

#include "core/broker.h"
#include "core/quotetable.h"

class VirtualBroker : public Broker
{
public:
	VirtualBroker(double startCash, const QuoteTable::Ptr& table);
	virtual ~VirtualBroker();

	virtual void submitOrder(const Order::Ptr& order);
	virtual void cancelOrder(const Order::Ptr& order);

	virtual void registerOrderCallback(const OrderCallback& callback);
	virtual void unregisterOrderCallback(const OrderCallback& callback);

	virtual Order::Ptr order(int localId);

	virtual std::list<std::string> accounts();

	virtual std::list<Position> positions();

private:
	void orderStateUpdated(const Order::Ptr& order);

private:
	std::list<Order::Ptr> m_pendingOrders;
	std::list<OrderCallback> m_orderCallbacks;
	std::map<std::string, int> m_portfolio;
	double m_cash;
	QuoteTable::Ptr m_table;
};

#endif /* CORE_BROKERS_VIRTUALBROKER_H_ */