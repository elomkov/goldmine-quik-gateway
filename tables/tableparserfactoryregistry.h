/*
 * tableparserfactoryregistry.h
 */

#ifndef TABLES_TABLEPARSERFACTORYREGISTRY_H_
#define TABLES_TABLEPARSERFACTORYREGISTRY_H_

#include <memory>
#include <functional>
#include "tableparser.h"
#include "core/datasink.h"

class TableParserFactoryRegistry
{
public:
	typedef std::function<TableParser::Ptr(const std::string& topic, const DataSink::Ptr& datasink)> Factory;
	typedef std::shared_ptr<TableParserFactoryRegistry> Ptr;

	TableParserFactoryRegistry();
	virtual ~TableParserFactoryRegistry();

	void registerFactory(const std::string& type, const Factory& f);

	TableParser::Ptr create(const std::string& type, const std::string& topic, const DataSink::Ptr& datasink);

private:
	std::map<std::string, Factory> m_factories;
};

#endif /* TABLES_TABLEPARSERFACTORYREGISTRY_H_ */
