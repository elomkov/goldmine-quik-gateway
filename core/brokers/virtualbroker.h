/*
 * virtualbroker.h
 */

#ifndef CORE_BROKERS_VIRTUALBROKER_H_
#define CORE_BROKERS_VIRTUALBROKER_H_

#include "core/broker.h"
#include "core/quotetable.h"

#include <boost/thread.hpp>

#include <ostream>
#include <istream>

class VirtualBroker : public Broker
{
public:
	VirtualBroker(double startCash, const QuoteTable::Ptr& table);
	virtual ~VirtualBroker();

	void save(std::ostream& stream);
	void load(std::istream& stream);

	virtual void submitOrder(const Order::Ptr& order);
	virtual void cancelOrder(const Order::Ptr& order);

	virtual void registerOrderCallback(const OrderCallback& callback);
	virtual void unregisterOrderCallback(const OrderCallback& callback);

	virtual void registerTradeCallback(const TradeCallback& callback);

	virtual Order::Ptr order(int localId);

	virtual std::list<std::string> accounts();

	virtual std::list<Position> positions();

private:
	void addPendingOrder(const Order::Ptr& order);
	void unsubscribeFromTickerIfNeeded(const std::string& ticker);
	void incomingTick(const std::string& ticker, const goldmine::Tick& tick);

private:
	void orderStateUpdated(const Order::Ptr& order);
	void emitTrade(const Trade& trade);
	void executeBuyAt(const Order::Ptr& order, const goldmine::decimal_fixed& price, uint64_t timestamp, uint32_t useconds);
	void executeSellAt(const Order::Ptr& order, const goldmine::decimal_fixed& price, uint64_t timestamp, uint32_t useconds);

private:
	std::list<Order::Ptr> m_pendingOrders;
	std::list<Order::Ptr> m_allOrders;
	std::list<OrderCallback> m_orderCallbacks;
	std::list<TradeCallback> m_tradeCallbacks;
	std::map<std::string, int> m_portfolio;
	double m_cash;
	QuoteTable::Ptr m_table;
	boost::recursive_mutex m_mutex;
};

#endif /* CORE_BROKERS_VIRTUALBROKER_H_ */
