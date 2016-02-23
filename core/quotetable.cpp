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
