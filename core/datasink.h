/*
 * datasink.h
 */

#ifndef CORE_DATASINK_H_
#define CORE_DATASINK_H_

#include <memory>
#include "goldmine/data.h"
#include <zmq.hpp>

class DataSink
{
public:
	typedef std::shared_ptr<DataSink> Ptr;

	DataSink(zmq::context_t& ctx);
	virtual ~DataSink();

	void connect(const std::string& ep);

	void incomingTick(const std::string& ticker, const goldmine::Tick& tick);

private:
	zmq::socket_t m_sink;
};

#endif /* CORE_DATASINK_H_ */
