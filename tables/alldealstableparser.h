/*
 * alldealstableparser.h
 */

#ifndef TABLES_ALLDEALSTABLEPARSER_H_
#define TABLES_ALLDEALSTABLEPARSER_H_

#include <memory>
#include "tableparser.h"
#include "core/datasink.h"

class AllDealsTableParser : public TableParser
{
public:
	typedef std::shared_ptr<AllDealsTableParser> Ptr;

	AllDealsTableParser(const std::string& topic, const DataSink::Ptr& datasink);
	virtual ~AllDealsTableParser();

	virtual bool acceptsTopic(const std::string& topic);

	virtual void incomingTable(const XlTable::Ptr& table);

private:
	bool schemaObtained() const;
	void obtainSchema(const XlTable::Ptr& table);

	void parseRow(int i, const XlTable::Ptr& table);

private:
	std::string m_topic;
	DataSink::Ptr m_datasink;
	std::vector<int> m_schema;
};

#endif /* TABLES_ALLDEALSTABLEPARSER_H_ */
