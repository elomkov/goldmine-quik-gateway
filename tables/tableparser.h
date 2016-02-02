/*
 * tableparser.h
 */

#ifndef TABLES_TABLEPARSER_H_
#define TABLES_TABLEPARSER_H_

#include "core/xltable/xltable.h"
#include "json.h"
#include <memory>

class TableParser
{
public:
	typedef std::shared_ptr<TableParser> Ptr;

	virtual ~TableParser() {}

	virtual bool acceptsTopic(const std::string& topic) = 0;

	virtual void incomingTable(const XlTable::Ptr& table) = 0;

	virtual void parseConfig(const Json::Value& root) = 0;
};

#endif /* TABLES_TABLEPARSER_H_ */
