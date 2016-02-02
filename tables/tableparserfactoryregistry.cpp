/*
 * tableparserfactoryregistry.cpp
 */

#include <tables/tableparserfactoryregistry.h>

TableParserFactoryRegistry::TableParserFactoryRegistry()
{
}

TableParserFactoryRegistry::~TableParserFactoryRegistry()
{
}

void TableParserFactoryRegistry::registerFactory(const std::string& type, const TableParserFactoryRegistry::Factory& f)
{
	m_factories[type] = f;
}

TableParser::Ptr TableParserFactoryRegistry::create(const std::string& type, const std::string& topic, const DataSink::Ptr& datasink)
{
	auto it = m_factories.find(type);
	if(it == m_factories.end())
		throw std::runtime_error("Unable to create parser with type: " + type);

	return it->second(topic, datasink);
}

