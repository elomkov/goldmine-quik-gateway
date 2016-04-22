/*
 * broker.h
 */

#ifndef CORE_BROKER_H_
#define CORE_BROKER_H_

#include <memory>
#include <list>
#include <string>
#include <map>
#include <functional>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "goldmine/data.h"

class Order
{
public:

	enum class OrderType
	{
		Market,
		Limit
	};

	enum class Operation
	{
		Buy,
		Sell
	};

	enum class State
	{
		Unsubmitted,
		Submitted,
		PartiallyExecuted,
		Executed,
		Cancelled,
		Rejected
	};

	typedef std::shared_ptr<Order> Ptr;

	Order(int clientAssignedId, const std::string& account, const std::string& security, double price, int amount, Operation operation, OrderType type);
	virtual ~Order();

	void updateState(State state);

	int localId() const { return m_id; }
	int clientAssignedId() const { return m_clientAssignedId; }

	std::string account() const { return m_account; }
	std::string security() const { return m_security; }
	double price() const { return m_price; }
	int amount() const { return m_amount; }
	Operation operation() const { return m_operation; }
	OrderType type() const { return m_type; }

	State state() const { return m_state; }

	std::string stringRepresentation() const;

	void setMessage(const std::string& message) { m_message = message; }

	std::string message() const { return m_message; }

private:
	int m_id;
	int m_clientAssignedId;

	std::string m_account;
	std::string m_security;
	double m_price;
	int m_amount;
	Operation m_operation;
	OrderType m_type;

	State m_state;

	std::string m_message;
};

struct Trade
{
	Trade() : orderId(0), price(0), amount(0), operation(Order::Operation::Buy) {}
	int orderId;
	goldmine::decimal_fixed price;
	int amount;
	Order::Operation operation;
	std::string account;
	std::string ticker;
	uint64_t timestamp;
	uint32_t useconds;
};

struct Position
{
	std::string security;
	int amount;
};

class Broker
{
public:
	typedef std::shared_ptr<Broker> Ptr;
	typedef std::function<void(Order::Ptr)> OrderCallback;
	typedef std::function<void(const Trade&)> TradeCallback;

	virtual ~Broker() {}

	virtual void submitOrder(const Order::Ptr& order) = 0;
	virtual void cancelOrder(const Order::Ptr& order) = 0;

	virtual void registerOrderCallback(const OrderCallback& callback) = 0;
	virtual void unregisterOrderCallback(const OrderCallback& callback) = 0;

	virtual void registerTradeCallback(const TradeCallback& callback) = 0;

	virtual Order::Ptr order(int localId) = 0;

	virtual std::list<std::string> accounts() = 0;

	virtual std::list<Position> positions() = 0;
};

#endif /* CORE_BROKER_H_ */
