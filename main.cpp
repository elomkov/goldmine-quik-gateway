
#include "log.h"

#include "core/quotesourceserver.h"
#include "ui/mainwindow.h"
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
#include "core/brokers/quikbroker/quikbroker.h"
#include "optionparser/optionparser.h"

#include <fstream>

struct Arg: public option::Arg
{
	static void printError(const char* msg1, const option::Option& opt, const char* msg2)
	{
		fprintf(stderr, "%s", msg1);
		fwrite(opt.name, opt.namelen, 1, stderr);
		fprintf(stderr, "%s", msg2);
	}

	static option::ArgStatus Unknown(const option::Option& option, bool msg)
	{
		if (msg) printError("Unknown option '", option, "'\n");
		return option::ARG_ILLEGAL;
	}

	static option::ArgStatus Required(const option::Option& option, bool msg)
	{
		if (option.arg != 0)
			return option::ARG_OK;

		if (msg) printError("Option '", option, "' requires an argument\n");
		return option::ARG_ILLEGAL;
	}

	static option::ArgStatus NonEmpty(const option::Option& option, bool msg)
	{
		if (option.arg != 0 && option.arg[0] != 0)
			return option::ARG_OK;

		if (msg) printError("Option '", option, "' requires a non-empty argument\n");
		return option::ARG_ILLEGAL;
	}

	static option::ArgStatus Numeric(const option::Option& option, bool msg)
	{
		char* endptr = 0;
		if (option.arg != 0 && strtol(option.arg, &endptr, 10)){};
		if (endptr != option.arg && *endptr == 0)
			return option::ARG_OK;

		if (msg) printError("Option '", option, "' requires a numeric argument\n");
		return option::ARG_ILLEGAL;
	}

	static option::ArgStatus Double(const option::Option& option, bool msg)
	{
		char* endptr = 0;
		double i = strtod(option.arg, &endptr);
		if (option.arg != 0 && i){};
		if (endptr != option.arg && *endptr == 0)
			return option::ARG_OK;

		if (msg) printError("Option '", option, "' requires a numeric argument\n");
		return option::ARG_ILLEGAL;
	}
};

enum MinerType { minerCandle, minerTime, minerZigzag };

enum  optionIndex { UNKNOWN,
	HELP,
	DEBUG,
	CONFIG_FILENAME
};
const option::Descriptor usage[] = {
{ UNKNOWN, 0,"", "",        Arg::Unknown, "USAGE: goldmine-quik-gateway.exe [options]\n\n"
                                          "Options:" },
{ HELP,    0,"", "help",    Arg::None,    "  \t--help  \tPrint usage and exit." },
{ DEBUG, 0, "d", "debug", Arg::None, "  \t-d, \t--debug"
											"  \tEnables debug output" },
{ CONFIG_FILENAME ,0,"","config-filename",Arg::Optional,"  --config-filename  \tSpecifies config filename (default is gqg.config)" },
{ 0, 0, 0, 0, 0, 0 } };

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
	argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
	option::Stats stats(usage, argc, argv);

	option::Option* options = new option::Option[stats.options_max];
	option::Option* buffer = new option::Option[stats.buffer_max];

	option::Parser parse(usage, argc, argv, options, buffer);

	if(parse.error())
		throw std::runtime_error("Unable to parse options");

	log_init("goldmine-quik-gateway.log", options[DEBUG] ? true : false);
	LOG(INFO) << "Goldmine quik gateway started, version " << VERSION_STRING;

	std::string configFilename = "gqg.config";
	if(options[CONFIG_FILENAME])
	{
		configFilename = options[CONFIG_FILENAME].arg;
	}
	std::ifstream incfg(configFilename.c_str());
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
	QuotesourceServer evloop(ctx, "tcp://*:5516", "inproc://tick-pipeline");

	createTableParsers(registry, root, evloop.dataImportServer(), evloop.datasink());

	evloop.start();

	auto quoteTable = std::make_shared<QuoteTable>();
	evloop.datasink()->setQuoteTable(quoteTable);
	auto broker = std::make_shared<VirtualBroker>(100000, quoteTable);

	{
		std::ifstream vbrokerState("vbroker.state", std::ios_base::in);
		if(vbrokerState.is_open())
			broker->load(vbrokerState);
	}

	BrokerServer brokerServer(ctx, "tcp://*:5520");
	brokerServer.addBroker(broker);
	std::map<std::string, std::string> config;
	config["dll_path"] = R"(C:\Program Files (x86)\Info\TRANS2QUIK.DLL)";
	config["quik_path"] = R"(C:\Program Files (x86)\Info)";
	brokerServer.addBroker(std::make_shared<QuikBroker>(config));

	brokerServer.start();

	MainWindow wnd;
	wnd.show(argc, argv);
	auto rc = Fl::run();
	evloop.stop();
	{
		std::ofstream vbrokerState("vbroker.state", std::ios_base::out | std::ios_base::trunc);
		if(vbrokerState.is_open())
			broker->save(vbrokerState);
		else
		{
			LOG(WARNING) << "Unable to save state";
		}

	}
}

