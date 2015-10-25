/*
 * tableparser.h
 */

#ifndef TABLES_TABLEPARSER_H_
#define TABLES_TABLEPARSER_H_

#include "core/xlparser.h"
#include <memory>

class TableParser
{
public:
	typedef std::shared_ptr<TableParser> Ptr;

	virtual ~TableParser() {}

	virtual bool acceptsTopic(const std::string& topic) = 0;

	virtual void incomingTable(int width, int height, const std::vector<XlParser::XlCell>& table) = 0;
};

#endif /* TABLES_TABLEPARSER_H_ */
