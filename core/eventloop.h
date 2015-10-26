/*
 * eventloop.h
 */

#ifndef CORE_EVENTLOOP_H_
#define CORE_EVENTLOOP_H_

#include <boost/thread.hpp>
#include "client.h"
#include "dataimportserver.h"
#include <zmq.hpp>
#include "datasink.h"

class EventLoop
{
public:
	EventLoop(zmq::context_t& ctx,
			const std::string& controlEp,
			const std::string& incomingPipeEp);
	virtual ~EventLoop();

	void start();
	void stop();

private:
	void run();

	void handleControlSocket();
	void handleIncomingPipe();

	void sendStreamPacket(const std::string& ticker, int datatype, void* packet, size_t packetSize);
	void sendPacketTo(const byte_array& peerId, const std::string& ticker, void* packet, size_t packetSize);
	void sendControlResponse(const byte_array& peerId);

	Client::Ptr getClient(const byte_array& peerId);
	void deleteClient(const byte_array& peerId);

	void handleControlCommand(const byte_array& peerId, uint8_t* buffer, size_t size);

private:
	bool m_run;
	boost::thread m_thread;
	DataImportServer::Ptr m_dataImportServer;
	std::vector<Client::Ptr> m_clients;

	zmq::socket_t m_control;
	std::string m_controlEndpoint;

	zmq::socket_t m_incomingPipe;
	std::string m_incomingPipeEndpoint;

	DataSink::Ptr m_datasink;
};

#endif /* CORE_EVENTLOOP_H_ */
