
#ifndef QUIKBROKER_H
#define QUIKBROKER_H

#include "core/broker.h"

#include "core/trans2quik/trans2quik.h"

#include <boost/thread.hpp>

#include <memory>
#include <map>
#include <string>

class QuikBroker : public Broker
{
public:
	using Ptr = std::shared_ptr<Broker>;

	QuikBroker(const std::map<std::string, std::string>& config);
	virtual ~QuikBroker();

	virtual void submitOrder(const Order::Ptr& order) override;
	virtual void cancelOrder(const Order::Ptr& order) override;

	virtual void registerOrderCallback(const OrderCallback& callback) override;
	virtual void unregisterOrderCallback(const OrderCallback& callback) override;

	virtual void registerTradeCallback(const TradeCallback& callback) override;

	virtual Order::Ptr order(int localId) override;

	virtual std::list<std::string> accounts() override;

	virtual std::list<Position> positions() override;

private:
	static void connectionStatusCallback(long event, long errorCode, LPSTR infoMessage);
	static void transactionReplyCallback(long result, long errorCode, long transactionReplyCode, DWORD transactionId, double orderNum, LPSTR transactionReplyMessage);
	static void orderStatusCallback(long mode, DWORD transactionId, double number,
			LPSTR classCode, LPSTR secCode, double price, long balance, double volume, long isBuy, long status, long orderDescriptor);
	static void tradeCallback(long mode, double number, double orderNumber, LPSTR classCode, double price, long quantity, double volume, long isSell, long tradeDescriptor);

	std::string makeTransactionStringForOrder(const Order::Ptr& order, int transactionId);

	void executeOrderStateCallbacks(const Order::Ptr& order);
	void executeTradeCallback(const Trade& trade);

private:
	static QuikBroker* m_instance;
	std::unique_ptr<Trans2QuikApi> m_quik;
	std::map<int, Order::Ptr> m_unsubmittedOrders;

	// yes, trans2quik api uses double to identify orders
	std::map<double, Order::Ptr> m_pendingOrders;
	std::list<Order::Ptr> m_retiredOrders;
	std::map<int, double> m_orderIdMap;
	boost::recursive_mutex m_mutex;

	std::vector<OrderCallback> m_orderCallbacks;
	std::vector<TradeCallback> m_tradeCallbacks;
};

#endif /* ifndef QUIKBROKER_H */
