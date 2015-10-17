/*
 * bio_types.h
 */

#ifndef BYTE_ARRAY_H_
#define BYTE_ARRAY_H_

#include <stdint.h>
#include <vector>
#include <ostream>
#include <stdexcept>

class StreamEndException : public std::runtime_error
{
public:
	StreamEndException(const std::string& message = std::string()) :
		std::runtime_error(message) {}
};

typedef std::vector<uint8_t> byte_array;

std::ostream& operator<<(std::ostream& out, const byte_array& arr);
byte_array operator+(const byte_array& arr1, const byte_array& arr2);
byte_array& operator+=(byte_array& arr1, const byte_array& arr2);
uint8_t checksum(const byte_array& arr);

byte_array byteArrayFromString(const std::string& str);
byte_array byteArrayFromFixedString(const std::string& str, int length);

#endif /* BYTE_ARRAY_H_ */
