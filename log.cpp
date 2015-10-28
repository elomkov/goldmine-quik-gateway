/*
 * log.cpp
 */

#include "log.h"


INITIALIZE_EASYLOGGINGPP

void log_init(const std::string& logFilename, bool debug)
{
	el::Configurations defaultConf;
	defaultConf.setToDefault();

	if(!debug)
	{
		defaultConf.set(el::Level::Global, el::ConfigurationType::Format, "%datetime %levshort %msg");
		defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
		defaultConf.set(el::Level::Trace, el::ConfigurationType::Enabled, "false");
	}
	else
	{
		defaultConf.set(el::Level::Global, el::ConfigurationType::Format, "%datetime %levshort %msg [%fbase:%line]");
		defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, "true");
		defaultConf.set(el::Level::Trace, el::ConfigurationType::Enabled, "true");
	}

	defaultConf.set(el::Level::Global, el::ConfigurationType::Filename, logFilename);


	el::Loggers::reconfigureAllLoggers(defaultConf);
}
