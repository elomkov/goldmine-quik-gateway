/*
 * broker.cpp
 */

#include "broker.h"
#include <atomic>
#include <sstream>

static std::atomic_int gs_id;

static std::string opString(Order::Operation op)
{
	if(op == Order::Operation::Buy)
		return "buy";
	else if(op == Order::Operation::Sell)
		return "sell";
	else
		return "???";
}

Order::Order(int clientAssignedId, const std::string& account, const std::string& security,
		double price, int amount, Operation operation, OrderType type) :
	m_id(gs_id.fetch_add(1)),
	m_clientAssignedId(clientAssignedId),
	m_account(account),
	m_security(security),
	m_price(price),
	m_amount(amount),
	m_operation(operation),
	m_type(type),
	m_state(State::Unsubmitted)
{

}
Order::~Order()
{

}

void Order::updateState(State state)
{
	m_state = state;
}

std::string Order::stringRepresentation() const
{
	std::stringstream ss;

	ss << "LocalID: " << m_id << "; ClientAssignedID: " << clientAssignedId() <<
			opString(m_operation) << " " << amount() << " of " << security();

	if(m_type == Order::OrderType::Limit)
		ss << " at " << m_price;

	return ss.str();
}
