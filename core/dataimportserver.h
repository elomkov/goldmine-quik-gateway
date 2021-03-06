/*
 * dataimportserver.h
 */

#ifndef CORE_DATAIMPORTSERVER_H_
#define CORE_DATAIMPORTSERVER_H_

#include <winsock2.h>
#include <windows.h>
#include <ddeml.h>
#include <string>
#include <memory>
#include <stdexcept>
#include "tables/tableparser.h"

class DataImportServer
{
public:
	typedef std::shared_ptr<DataImportServer> Ptr;

	DataImportServer(const std::string& serverName, const std::string& topicName);
	virtual ~DataImportServer();

	void registerTableParser(const TableParser::Ptr& parser);

public:
	HDDEDATA ddeCallback(UINT type, UINT fmt, HCONV hConv, HSZ hsz1, HSZ hsz2, HDDEDATA hData, ULONG_PTR dwData1, ULONG_PTR dwData2);

private:
	bool parseIncomingData(const std::string& topic, HDDEDATA hData);

private:
	HSZ m_appName;
	HSZ m_topicName;
	long unsigned int m_instanceId;
	std::vector<TableParser::Ptr> m_tableParsers;
};

#endif /* CORE_DATAIMPORTSERVER_H_ */
