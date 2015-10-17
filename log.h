/*
 * log.h
 *
 *  Created on: 13 окт. 2015 г.
 *      Author: todin
 */

#ifndef LOG_H_
#define LOG_H_

#include "easylogging/easylogging++.h"

void log_init(const std::string& logFilename, bool debug);

#endif /* LOG_H_ */
