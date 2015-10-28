/*
 * currentparametertableparser.cpp
 */

#include "currentparametertableparser.h"
#include "log.h"

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

void CurrentParameterTableParser::incomingTable(int width, int height, const std::vector<XlParser::XlCell>& table)
{
	if(width < 8)
	{
		LOG(WARNING) << "Truncated table: not enough columns, should be 8, got " << width;
		return;
	}

	try
	{
		for(int row = 0; row < height; row++)
		{
			int rowStart = row * width;
			parseRow(rowStart, table);
		}
	}
	catch(const std::exception& e)
	{
		LOG(WARNING) << "Unable to parse incoming table, exception thrown: " << e.what();
	}
}

void CurrentParameterTableParser::parseRow(int rowStart, const std::vector<XlParser::XlCell>& table)
{
	auto contractCode = boost::get<std::string>(table[rowStart]);
	auto bidPrice = boost::get<double>(table[rowStart + 1]);
	auto askPrice = boost::get<double>(table[rowStart + 2]);
	auto lastPrice = boost::get<double>(table[rowStart + 3]);
	auto openInterest = boost::get<double>(table[rowStart + 4]);
	auto totalBid = boost::get<double>(table[rowStart + 5]);
	auto totalAsk = boost::get<double>(table[rowStart + 6]);
	auto cumulativeVolume = boost::get<double>(table[rowStart + 7]);

	long volume = 1;
	long lastVolume = m_volumes[contractCode];
	if(lastVolume == 0)
	{
		lastVolume = cumulativeVolume;
	}
	else
	{
		volume = cumulativeVolume - lastVolume;
		m_volumes[contractCode] = cumulativeVolume;
	}

	double delta = lastPrice - m_prices[contractCode];
	m_prices[contractCode] = lastPrice;

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
