/*
 * data.h
 *
 *  Created on: 12 окт. 2015 г.
 *      Author: todin
 */

#ifndef GOLDMINE_DATA_H_
#define GOLDMINE_DATA_H_

#include <cstdint>
#include <cmath>

namespace goldmine
{

	enum class MessageType
	{
		Control = 0x01,
		Stream = 0x02,
		StreamCredit = 0x03
	};

	enum class PacketType
	{
		Tick = 0x01,
		Summary = 0x02,
		Event = 0x03
	};

	enum class EventId
	{
		StreamEnd = 0x01
	};

	enum class Datatype
	{
		Price = 0x01,
		OpenInterest = 0x03,
		BestBid = 0x04,
		BestOffset = 0x05,
		Depth = 0x06,
		TheoryPrice = 0x07,
		Volatility = 0x08,
		TotalSupply = 0x09,
		TotalDemand = 0x0a
	};

	struct decimal_fixed
	{
		int64_t value;
		int32_t fractional; // 1e-9 parts

		decimal_fixed(int64_t int_part, int32_t nano) : value(int_part), fractional(nano)
		{
		}

		decimal_fixed(double v) : value(::floor(v)), fractional(v - floor(v))
		{

		}

		double toDouble() const
		{
			return (double)value + (double)fractional / 1e9;
		}
	} __attribute__((__packed__));

	struct Tick
	{
		uint32_t packet_type; // = 0x01
		uint64_t timestamp;
		uint32_t useconds;
		uint32_t datatype;
		decimal_fixed value;
		int32_t volume;
	} __attribute__((__packed__));

	struct Summary
	{
		uint32_t packet_type; // = 0x02
		uint64_t timestamp;
		uint32_t useconds;
		uint32_t datatype;
		decimal_fixed open;
		decimal_fixed high;
		decimal_fixed low;
		decimal_fixed close;
		int32_t volume;
		uint32_t summary_period_seconds;
	} __attribute__((__packed__));

	struct Event
	{
		uint32_t packet_type; // = 0x03
		uint32_t event_id;
	} __attribute__((__packed__));
}

#endif /* GOLDMINE_DATA_H_ */
