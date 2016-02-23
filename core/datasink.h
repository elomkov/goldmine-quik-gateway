/*
 * datasink.h
 */

#ifndef CORE_DATASINK_H_
#define CORE_DATASINK_H_

#include <memory>
#include "goldmine/data.h"
#include <zmq.hpp>
#include "core/quotetable.h"

class DataSink
{
public:
	typedef std::shared_ptr<DataSink> Ptr;

	DataSink(zmq::context_t& ctx);
	virtual ~DataSink();

	void connect(const std::string& ep);

	void incomingTick(const std::string& ticker, const goldmine::Tick& tick);

	void setQuoteTable(const QuoteTable::Ptr& table);

private:
	zmq::socket_t m_sink;
	QuoteTable::Ptr m_table;
};

#endif /* CORE_DATASINK_H_ */
