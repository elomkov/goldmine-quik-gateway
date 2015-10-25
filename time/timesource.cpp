/*
 * timesource.cpp
 *
 *  Created on: 20 окт. 2015 г.
 *      Author: todin
 */

#include <time/timesource.h>
#include <ctime>

TimeSource::TimeSource()
{
}

TimeSource::~TimeSource()
{
}

std::pair<uint64_t, uint32_t> TimeSource::preciseTimestamp()
{
	time_t t = time(0);
	return std::make_pair<uint64_t, uint32_t>(t, 0);
}
