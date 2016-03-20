
#include "catch.hpp"

#include "core/quotesourceserver.h"
#include "core/datasink.h"

#include "goldmine/data.h"

#include <zmq.hpp>
#include "json.h"

static void sendCommand(zmq::socket_t& socket, const Json::Value& root)
{
	zmq::message_t msgDelimiter;

	uint32_t messageType = (uint32_t)goldmine::MessageType::Control;
	zmq::message_t msgMessageType((void*)(&messageType), 4);

	Json::FastWriter writer;
	auto jsonRoot = writer.write(root);

	zmq::message_t msgRequest(jsonRoot.data(), jsonRoot.size());

	socket.send(msgDelimiter, ZMQ_SNDMORE);
	socket.send(msgMessageType, ZMQ_SNDMORE);
	socket.send(msgRequest);
}

static void sendCredits(zmq::socket_t& socket, int credits = 1)
{
	zmq::message_t msgDelimiter;
	socket.send(msgDelimiter, ZMQ_SNDMORE);

	if(credits == 1)
	{
		uint8_t messageType = (uint8_t)goldmine::MessageType::StreamCredit;
		zmq::message_t msgRequest((void*)(&messageType), 1);
		socket.send(msgRequest);
	}
	else
	{
		uint8_t data[8]; 
		data[0] = (uint8_t)goldmine::MessageType::StreamCredit;
		data[1] = (credits >> 0) & 0xff;
		data[2] = (credits >> 8) & 0xff;
		data[3] = (credits >> 16) & 0xff;
		data[4] = (credits >> 24) & 0xff;
		zmq::message_t msgRequest((void*)data, 5);
		socket.send(msgRequest);
	}
}

static Json::Value getControlResponse(zmq::socket_t& socket)
{
	zmq::message_t msgDelimiter;
	socket.recv(&msgDelimiter);

	zmq::message_t msgMessageType;
	socket.recv(&msgMessageType);

	uint8_t* messageType = (uint8_t*)msgMessageType.data();
	REQUIRE(*messageType == (int)goldmine::MessageType::Control);

	zmq::message_t msgResponse;
	socket.recv(&msgResponse);

	std::string json((char*)msgResponse.data(), msgResponse.size());
	Json::Reader r;
	Json::Value root;
	r.parse(json, root);
	return root;
}

// Returns ticker and data packet length
static std::pair<std::string, int> readStreamPacket(zmq::socket_t& socket, char* buffer, int bufferLength)
{
	zmq::message_t msgDelimiter;
	socket.recv(&msgDelimiter);
	zmq::message_t msgPacketType;
	socket.recv(&msgPacketType);
	uint8_t* packetType = reinterpret_cast<uint8_t*>(msgPacketType.data());
	REQUIRE(*packetType == (int)goldmine::MessageType::Stream);

	zmq::message_t msgTicker;
	zmq::message_t msgTickData;
	socket.recv(&msgTicker);
	std::string recvdTicker = std::string(reinterpret_cast<char*>(msgTicker.data()), msgTicker.size());
	socket.recv(&msgTickData);
	REQUIRE(bufferLength >= msgTickData.size());
	memcpy(buffer, msgTickData.data(), msgTickData.size());
	return std::make_pair(recvdTicker, (int)msgTickData.size());
}

TEST_CASE("QuotesourceServer", "[core][quotesource][server]")
{
	zmq::context_t ctx;
	QuotesourceServer server(ctx, "inproc://quotesource-server", "inproc://incoming-ticks");
	zmq::socket_t client(ctx, ZMQ_DEALER);
	client.connect("inproc://quotesource-server");

	auto datasink = std::make_shared<DataSink>(ctx);
	datasink->connect("inproc://incoming-ticks");

	server.start();

	SECTION("Stream request: one ticker, ticks, only price")
	{
		Json::Value root;
		root["command"] = "start";
		root["tickers"].append("t:TEST");

		sendCommand(client, root);

		getControlResponse(client);

		SECTION("One tick")
		{
			// We expect only one packet, so send only one credit
			sendCredits(client);

			// Pass tick to datasink
			goldmine::Tick tick;
			tick.timestamp = 12;
			tick.useconds = 0;
			tick.datatype = (uint32_t)goldmine::Datatype::Price;
			tick.value = goldmine::decimal_fixed(42.0);
			tick.volume = 40;
			datasink->incomingTick("TEST", tick);

			// Read tick data into tickBuffer
			std::array<char, 1024> tickBuffer;
			std::string recvdTicker;
			int dataPacketSize;
			std::tie(recvdTicker, dataPacketSize) = readStreamPacket(client, tickBuffer.data(), tickBuffer.size());

			REQUIRE(recvdTicker == "TEST");
			REQUIRE(dataPacketSize == sizeof(goldmine::Tick));
			goldmine::Tick* recvedTick = reinterpret_cast<goldmine::Tick*>(tickBuffer.data());
			REQUIRE(*recvedTick == tick);
		}

		SECTION("Two ticks in different packets")
		{
			// Send two credits, because we expect 2 packets
			SECTION("Send credits")
			{
				SECTION("Send credits via single-credit packet")
				{
					sendCredits(client);
					sendCredits(client);
				}

				SECTION("Send credits via multi-credit packet")
				{
					sendCredits(client, 2);
				}
			}

			// Pass one tick to datasink
			goldmine::Tick tick1;
			tick1.timestamp = 12;
			tick1.useconds = 0;
			tick1.datatype = (uint32_t)goldmine::Datatype::Price;
			tick1.value = goldmine::decimal_fixed(42.0);
			tick1.volume = 40;
			datasink->incomingTick("TEST", tick1);

			// Pass another tick to datasink
			goldmine::Tick tick2;
			tick2.timestamp = 14;
			tick2.useconds = 0;
			tick2.datatype = (uint32_t)goldmine::Datatype::Price;
			tick2.value = goldmine::decimal_fixed(44.0);
			tick2.volume = 41;
			datasink->incomingTick("TEST", tick2);

			std::array<char, 1024> tickBuffer;
			std::string recvdTicker;
			int dataPacketSize;

			// Read first tick
			std::tie(recvdTicker, dataPacketSize) = readStreamPacket(client, tickBuffer.data(), tickBuffer.size());
			REQUIRE(recvdTicker == "TEST");
			REQUIRE(dataPacketSize == sizeof(goldmine::Tick));
			goldmine::Tick* recvedTick = reinterpret_cast<goldmine::Tick*>(tickBuffer.data());
			REQUIRE(*recvedTick == tick1);

			// Read second tick
			std::tie(recvdTicker, dataPacketSize) = readStreamPacket(client, tickBuffer.data(), tickBuffer.size());
			REQUIRE(recvdTicker == "TEST");
			REQUIRE(dataPacketSize == sizeof(goldmine::Tick));
			recvedTick = reinterpret_cast<goldmine::Tick*>(tickBuffer.data());
			REQUIRE(*recvedTick == tick2);
		}
	}

	server.stop();
}

