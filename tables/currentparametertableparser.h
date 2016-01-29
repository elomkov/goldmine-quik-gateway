/*
 * currentparametertableparser.h
 */

#ifndef TABLES_CURRENTPARAMETERTABLEPARSER_H_
#define TABLES_CURRENTPARAMETERTABLEPARSER_H_

#include "tables/tableparser.h"
#include <unordered_map>
#include <memory>
#include "core/datasink.h"
#include "time/timesource.h"

class CurrentParameterTableParser : public TableParser
{
public:
	typedef std::shared_ptr<CurrentParameterTableParser> Ptr;

	CurrentParameterTableParser(const std::string& topic, const DataSink::Ptr& datasink, const TimeSource::Ptr& timesource);
	virtual ~CurrentParameterTableParser();

	virtual bool acceptsTopic(const std::string& topic);
	virtual void incomingTable(const XlTable::Ptr& table);

private:
	void parseRow(int row, const XlTable::Ptr& table);

private:
	std::string m_topic;
	std::unordered_map<std::string, unsigned long> m_volumes;
	std::unordered_map<std::string, double> m_prices;
	std::unordered_map<std::string, double> m_bids;
	std::unordered_map<std::string, double> m_asks;

	DataSink::Ptr m_datasink;
	TimeSource::Ptr m_timesource;
};

#endif /* TABLES_CURRENTPARAMETERTABLEPARSER_H_ */
