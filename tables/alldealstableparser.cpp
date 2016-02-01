/*
 * alldealstableparser.cpp
 */

#include <cstdio>
#include "alldealstableparser.h"
#include "log.h"

enum ColumnId
{
	ClassCode = 0,
	Code,
	Date,
	TradeTime,
	TradeTimeMsec,
	Price,
	Quantity,
	BuySell,
	MaxId
};
static std::vector<std::string> gs_columnNames = {
		"CLASSCODE",
		"SECCODE",
		"TRADEDATE",
		"TRADETIME",
		"TRADETIME_MSEC",
		"PRICE",
		"QTY",
		"BUYSELL" };

static int indexOf(const std::string& code)
{
	auto it = std::find(gs_columnNames.begin(), gs_columnNames.end(), code);
	if(it != gs_columnNames.end())
		return std::distance(gs_columnNames.begin(), it);
	return -1;
}

static time_t _mkgmtime(const struct tm *tm)
{
    // Month-to-day offset for non-leap-years.
    static const int month_day[12] =
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

    // Most of the calculation is easy; leap years are the main difficulty.
    int month = tm->tm_mon % 12;
    int year = tm->tm_year + tm->tm_mon / 12;
    if (month < 0) {   // Negative values % 12 are still negative.
        month += 12;
        --year;
    }

    // This is the number of Februaries since 1900.
    const int year_for_leap = (month > 1) ? year + 1 : year;

    time_t rt = tm->tm_sec                             // Seconds
        + 60 * (tm->tm_min                          // Minute = 60 seconds
        + 60 * (tm->tm_hour                         // Hour = 60 minutes
        + 24 * (month_day[month] + tm->tm_mday - 1  // Day = 24 hours
        + 365 * (year - 70)                         // Year = 365 days
        + (year_for_leap - 69) / 4                  // Every 4 years is     leap...
        - (year_for_leap - 1) / 100                 // Except centuries...
        + (year_for_leap + 299) / 400)));           // Except 400s.
    return rt < 0 ? -1 : rt;
}

AllDealsTableParser::AllDealsTableParser(const std::string& topic, const DataSink::Ptr& datasink) :
	m_topic(topic), m_datasink(datasink)
{
}

AllDealsTableParser::~AllDealsTableParser()
{
}

bool AllDealsTableParser::acceptsTopic(const std::string& topic)
{
	return m_topic == topic;
}

void AllDealsTableParser::incomingTable(const XlTable::Ptr& table)
{
	if(!schemaObtained())
		obtainSchema(table);

	for(int i = 0; i < table->height(); i++)
	{
		parseRow(i, table);
	}
}

void AllDealsTableParser::parseRow(int i, const XlTable::Ptr& table)
{
	std::string code;
	try
	{
		auto contractClassCode = boost::get<std::string>(table->get(i, m_schema[ClassCode]));
		auto contractCode = boost::get<std::string>(table->get(i, m_schema[Code]));
		code = contractClassCode + "#" + contractCode;
	}
	catch(const boost::bad_get& e)
	{
		LOG(WARNING) << "Unable to parse contract code from table";
		return;
	}

	std::string date = boost::get<std::string>(table->get(i, m_schema[Date]));
	std::string time = boost::get<std::string>(table->get(i, m_schema[TradeTime]));
	double timeMsec = boost::get<double>(table->get(i, m_schema[TradeTimeMsec]));

	struct tm t;
	std::sscanf(date.c_str(), "%d.%d.%d", &t.tm_mday, &t.tm_mon, &t.tm_year);
	t.tm_year -= 1900;
	t.tm_mon -= 1;


	const char* tstr = time.c_str();
	t.tm_hour = tstr[0] * 10 + tstr[1];
	t.tm_min = tstr[3] * 10 + tstr[4];
	t.tm_sec = tstr[6] * 10 + tstr[7];

	goldmine::Tick tick;
	tick.timestamp = mktime(&t);
	tick.useconds = timeMsec;
	tick.datatype = (int)goldmine::Datatype::Price;
	tick.value = boost::get<double>(table->get(i, m_schema[Price]));
	std::string buysell = boost::get<std::string>(table->get(i, m_schema[BuySell]));
	tick.volume = boost::get<double>(table->get(i, m_schema[Quantity]));
	if(buysell.size() > 3) // If "Sell"
		tick.volume = -tick.volume;

	m_datasink->incomingTick(code, tick);
}

bool AllDealsTableParser::schemaObtained() const
{
	return !m_schema.empty();
}

void AllDealsTableParser::obtainSchema(const XlTable::Ptr& table)
{
	m_schema.resize(MaxId, -1);
	for(int i = 0; i < table->width(); i++)
	{
		std::string header;

		try
		{
			header = boost::get<std::string>(table->get(0, i));
		}
		catch(const boost::bad_get& e)
		{
			// Skip silently
		}
		int index = indexOf(header);
		m_schema[index] = i;

		LOG(DEBUG) << "[" << header << "] -> " << index;
	}
}
