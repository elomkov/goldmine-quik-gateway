/*
 * currentparametertableparser.cpp
 */

#include "currentparametertableparser.h"
#include "log.h"
#include <cstdlib>

// Parameter table parser:
// Required schema:
// Without rows, without columns:
// Code, Bid, Ask, Last price, Open Interest, Total bid, Total ask, Volume

CurrentParameterTableParser::CurrentParameterTableParser(const std::string& topic,
		const DataSink::Ptr& datasink,
		const TimeSource::Ptr& timesource) : m_topic(topic),
	m_datasink(datasink),
	m_timesource(timesource)
{
}

CurrentParameterTableParser::~CurrentParameterTableParser()
{
}

bool CurrentParameterTableParser::acceptsTopic(const std::string& topic)
{
	return topic == m_topic;
}

void CurrentParameterTableParser::incomingTable(const XlTable::Ptr& table)
{
	if(table->width() < 8)
	{
		LOG(WARNING) << "Truncated table: not enough columns, should be 8, got " << table->width();
		return;
	}

	try
	{
		for(int row = 0; row < table->height(); row++)
		{
			parseRow(row, table);
		}
	}
	catch(const std::exception& e)
	{
		LOG(WARNING) << "Unable to parse incoming table, exception thrown: " << e.what();
	}
}

void CurrentParameterTableParser::parseRow(int row, const XlTable::Ptr& table)
{
	auto contractCode = boost::get<std::string>(table->get(row, 0));
	auto bidPrice = boost::get<double>(table->get(row, 1));
	auto askPrice = boost::get<double>(table->get(row, 2));
	auto lastPrice = boost::get<double>(table->get(row, 3));
	auto openInterest = boost::get<double>(table->get(row, 4));
	auto totalBid = boost::get<double>(table->get(row, 5));
	auto totalAsk = boost::get<double>(table->get(row, 6));
	auto cumulativeVolume = boost::get<double>(table->get(row, 7));

	long volume = 1;
	long lastVolume = m_volumes[contractCode];
	if(lastVolume == 0)
	{
		lastVolume = cumulativeVolume;
	}
	else
	{
		volume = cumulativeVolume - lastVolume;
	}
	m_volumes[contractCode] = cumulativeVolume;

	double delta = 0;

	if(lastPrice == bidPrice)
	{
		delta = -1;
	}
	else if(lastPrice == askPrice)
	{
		delta = 1;
	}
	else if(lastPrice <= m_bids[contractCode])
	{
		delta = -1;
	}
	else if(lastPrice >= m_asks[contractCode])
	{
		delta = 1;
	}
	else if(lastPrice == m_prices[contractCode])
	{
		// Make a random guess
		delta = (rand() % 2) == 0 ? 1 : -1;
	}
	else
	{
		delta = lastPrice - m_prices[contractCode];
	}

	m_prices[contractCode] = lastPrice;
	m_bids[contractCode] = bidPrice;
	m_asks[contractCode] = askPrice;

	auto currentTime = m_timesource->preciseTimestamp();

	goldmine::Tick tick;
	tick.timestamp = currentTime.first;
	tick.useconds = currentTime.second;

	tick.datatype = (int)goldmine::Datatype::Price;
	tick.value = lastPrice;
	tick.volume = delta > 0 ? volume : -volume;
	m_datasink->incomingTick(contractCode, tick);

	tick.datatype = (int)goldmine::Datatype::BestBid;
	tick.value = bidPrice;
	tick.volume = 0;
	m_datasink->incomingTick(contractCode, tick);

	tick.datatype = (int)goldmine::Datatype::BestOffer;
	tick.value = askPrice;
	tick.volume = 0;
	m_datasink->incomingTick(contractCode, tick);

	tick.datatype = (int)goldmine::Datatype::TotalDemand;
	tick.value = totalBid;
	tick.volume = 0;
	m_datasink->incomingTick(contractCode, tick);

	tick.datatype = (int)goldmine::Datatype::TotalSupply;
	tick.value = totalAsk;
	tick.volume = 0;
	m_datasink->incomingTick(contractCode, tick);
}
