/*
 * datasink.cpp
 */

#include "core/datasink.h"
#include "log.h"
#include "goldmine/data.h"

DataSink::DataSink(zmq::context_t& ctx) : m_sink(ctx, ZMQ_PUSH)
{
}

DataSink::~DataSink()
{
}

void DataSink::connect(const std::string& ep)
{
	m_sink.connect(ep);
}

void DataSink::incomingTick(const std::string& ticker, const goldmine::Tick& tick)
{
	LOG(DEBUG) << "Incoming tick: " << ticker << "; " << tick.value.toDouble() << "(" << tick.datatype << ")";

	zmq::message_t msgTicker(ticker.size());
	memcpy(msgTicker.data(), ticker.c_str(), ticker.size());

	zmq::message_t msgDatatype(4);
	*((uint32_t*)msgDatatype.data()) = tick.datatype;

	zmq::message_t msgPacket(sizeof(tick));
	memcpy(msgPacket.data(), reinterpret_cast<const void*>(&tick), sizeof(tick));

	m_sink.send(msgTicker, ZMQ_SNDMORE);
	m_sink.send(msgDatatype, ZMQ_SNDMORE);
	m_sink.send(msgPacket, 0);
}
