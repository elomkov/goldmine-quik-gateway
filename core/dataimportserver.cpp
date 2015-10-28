/*
 * dataimportserver.cpp
 *
 *  Created on: 15 окт. 2015 г.
 *      Author: todin
 */

#include "dataimportserver.h"
#include "log.h"
#include "core/xlparser.h"

#include <boost/scope_exit.hpp>
#include <iomanip>

using namespace boost;

static DataImportServer* gs_server;

HDDEDATA theDdeCallback(UINT type, UINT fmt, HCONV hConv, HSZ hsz1, HSZ hsz2, HDDEDATA hData, ULONG_PTR dwData1, ULONG_PTR dwData2)
{
	return gs_server->ddeCallback(type, fmt, hConv, hsz1, hsz2, hData, dwData1, dwData2);
}

DataImportServer::DataImportServer(const std::string& serverName, const std::string& topicName) : m_appName(0),
	m_topicName(0),
	m_instanceId(0)
{
	gs_server = this;
	if(DdeInitialize(&m_instanceId, (PFNCALLBACK)theDdeCallback, APPCLASS_STANDARD, 0))
		throw std::runtime_error("Unable to initialize DDE server");

	m_appName = DdeCreateStringHandle(m_instanceId, serverName.c_str(), 0);
	if(!m_appName)
		throw std::runtime_error("Unable to create string handle");
	m_topicName = DdeCreateStringHandle(m_instanceId, topicName.c_str(), 0);
	if(!m_topicName)
		throw std::runtime_error("Unable to create string handle");

	if(!DdeNameService(m_instanceId, m_appName, NULL, DNS_REGISTER))
		throw std::runtime_error("Unable to register DDE server");
}

DataImportServer::~DataImportServer()
{
}

void DataImportServer::registerTableParser(const TableParser::Ptr& parser)
{
	if(std::find(m_tableParsers.begin(), m_tableParsers.end(), parser) == m_tableParsers.end())
		m_tableParsers.push_back(parser);
}

HDDEDATA DataImportServer::ddeCallback(UINT type, UINT fmt, HCONV hConv, HSZ hsz1, HSZ hsz2, HDDEDATA hData, ULONG_PTR dwData1, ULONG_PTR dwData2)
{
	LOG(DEBUG) << "Incoming DDE message: " << type;
	switch(type)
	{
	case XTYP_CONNECT:
		{
			char topicBuf[256];
			char appBuf[256];
			DdeQueryString(m_instanceId, hsz1, topicBuf, 256, CP_WINANSI);
			DdeQueryString(m_instanceId, hsz2, appBuf, 256, CP_WINANSI);
			LOG(INFO) << "Client connect: " << appBuf << " : " << topicBuf;
			if(!DdeCmpStringHandles(hsz2, m_appName))
				return (HDDEDATA)TRUE;
			else
				return (HDDEDATA)FALSE;
		}
		break;
	case XTYP_POKE:
		{
			LOG(DEBUG) << "Client poke";

			char topicBuf[256];
			DdeQueryString(m_instanceId, hsz1, topicBuf, 256, CP_WINANSI);
			std::string topic(topicBuf);

			parseIncomingData(topic, hData);

			LOG(DEBUG) << "Incoming table parsed";

			return (HDDEDATA)DDE_FACK;
		}

	default:
		return NULL;
	}
}

bool DataImportServer::parseIncomingData(const std::string& topic, HDDEDATA hData)
{
	DWORD dataSize = 0;
	BYTE* data = DdeAccessData(hData, &dataSize);
	if(!data)
		return false;

	BOOST_SCOPE_EXIT(hData)
	{
		DdeUnaccessData(hData);
	} BOOST_SCOPE_EXIT_END;

	try
	{
		XlParser parser;
		parser.parse(data, dataSize);

		LOG(DEBUG) << "Incoming table: " << parser.width() << "x" << parser.height();

		for(const auto& tp : m_tableParsers)
		{
			if(tp->acceptsTopic(topic))
				tp->incomingTable(parser.width(), parser.height(), parser.getData());
		}
	}
	catch(const std::exception& e)
	{
		LOG(WARNING) << "Unable to parse incoming XlTable: " << e.what();
	}


	return true;
}
