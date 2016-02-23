
#include "log.h"

#include "ui/mainwindow.h"
#include "core/eventloop.h"
#include <zmq.hpp>
#include "goldmine/data.h"
#include "version.h"
#include "json.h"
#include "tables/tableparserfactoryregistry.h"
#include <functional>
#include "time/timesource.h"
#include "tables/currentparametertableparser.h"
#include "tables/alldealstableparser.h"
#include "core/brokerserver.h"
#include "core/quotetable.h"
#include "core/brokers/virtualbroker.h"

static void createTableParsers(const TableParserFactoryRegistry::Ptr& registry,
		const Json::Value& root,
		const DataImportServer::Ptr& dataImportServer,
		const DataSink::Ptr& sink)
{
	auto parsers = root["parsers"];
	for(auto it = parsers.begin(); it != parsers.end(); ++it)
	{
		auto value = *it;
		auto type = value["type"].asString();
		auto topic = value["topic"].asString();

		auto parser = registry->create(type, topic, sink);
		dataImportServer->registerTableParser(parser);

	}
}

int main(int argc, char** argv)
{
	log_init("goldmine-quik-gateway.log", false);
	LOG(INFO) << "Goldmine quik gateway started, version " << VERSION_STRING;

	std::ifstream incfg("gqg.config");
	Json::Reader configReader;
	Json::Value root;
	if(!configReader.parse(incfg, root))
	{
		LOG(WARNING) << "Unable to parse config";
		return 1;
	}

	auto timesource = std::make_shared<TimeSource>();
	auto registry = std::make_shared<TableParserFactoryRegistry>();
	registry->registerFactory("current_parameters_table",
			[&](const std::string& topic, const DataSink::Ptr& sink)
			{ return createCurrentParameterTableParser(topic, sink, timesource); });

	registry->registerFactory("all_deals_table",
				[&](const std::string& topic, const DataSink::Ptr& sink)
				{ return createAllDealsTableParser(topic, sink); });

	zmq::context_t ctx;
	EventLoop evloop(ctx, "tcp://*:5516", "inproc://tick-pipeline");

	createTableParsers(registry, root, evloop.dataImportServer(), evloop.datasink());

	evloop.start();

	auto quoteTable = std::make_shared<QuoteTable>();
	evloop.datasink()->setQuoteTable(quoteTable);
	auto broker = std::make_shared<VirtualBroker>(100000, quoteTable);
	BrokerServer brokerServer(ctx, "tcp://*:5520", broker);

	brokerServer.start();

	MainWindow wnd;
	wnd.show(argc, argv);
	auto rc = Fl::run();
	evloop.stop();
}

