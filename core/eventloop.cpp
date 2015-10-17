/*
 * eventloop.cpp
 *
 *  Created on: 12 окт. 2015 г.
 *      Author: todin
 */

#include "eventloop.h"

EventLoop::EventLoop() : m_run(false)
{
	m_dataImportServer = std::make_shared<DataImportServer>("gold", "default");
}

EventLoop::~EventLoop()
{
}

void EventLoop::start()
{
	m_thread = boost::thread(std::bind(&EventLoop::run, this));
}

void EventLoop::stop()
{
	m_run = false;
	m_thread.join();
}

void EventLoop::run()
{
	m_run = true;
}

