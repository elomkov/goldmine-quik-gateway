/*
 * broker.cpp
 */

#include "broker.h"
#include <atomic>

static std::atomic_int gs_id;

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
