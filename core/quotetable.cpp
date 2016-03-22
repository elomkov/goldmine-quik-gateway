/*
 * quotetable.cpp
 */

#include <core/quotetable.h>

QuoteTable::QuoteTable()
{
}

QuoteTable::~QuoteTable()
{
}

void QuoteTable::updateQuote(const std::string& ticker, const goldmine::Tick& tick)
{
	m_table[std::make_pair(ticker, goldmine::Datatype(tick.datatype))] = tick;
	if(m_enabledTickers.find(ticker) != m_enabledTickers.end())
		m_callback(ticker, tick);
}

goldmine::Tick QuoteTable::lastQuote(const std::string& ticker, goldmine::Datatype datatype)
{
	auto it = m_table.find(std::make_pair(ticker, datatype));
	if(it != m_table.end())
	{
		return it->second;
	}
	return goldmine::Tick();
}

void QuoteTable::setTickCallback(const TickCallback& callback)
{
	m_callback = callback;
}

void QuoteTable::enableTicker(const std::string& ticker)
{
	m_enabledTickers.insert(ticker);
}

void QuoteTable::disableTicker(const std::string& ticker)
{
	auto it = m_enabledTickers.find(ticker);
	if(it != m_enabledTickers.end())
		m_enabledTickers.erase(it);
}

