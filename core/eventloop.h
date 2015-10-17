/*
 * eventloop.h
 */

#ifndef CORE_EVENTLOOP_H_
#define CORE_EVENTLOOP_H_

#include <boost/thread.hpp>
#include "dataimportserver.h"

class EventLoop
{
public:
	EventLoop();
	virtual ~EventLoop();

	void start();
	void stop();

private:
	void run();

private:
	bool m_run;
	boost::thread m_thread;
	DataImportServer::Ptr m_dataImportServer;
};

#endif /* CORE_EVENTLOOP_H_ */
