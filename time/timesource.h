/*
 * timesource.h
 *
 *  Created on: 20 окт. 2015 г.
 *      Author: todin
 */

#ifndef TIME_TIMESOURCE_H_
#define TIME_TIMESOURCE_H_

#include <memory>
#include <utility>

class TimeSource
{
public:
	typedef std::shared_ptr<TimeSource> Ptr;

	TimeSource();
	virtual ~TimeSource();

	std::pair<uint64_t, uint32_t> preciseTimestamp();
};

#endif /* TIME_TIMESOURCE_H_ */
