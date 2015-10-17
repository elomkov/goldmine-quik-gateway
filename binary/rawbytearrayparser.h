/*
 * rawbytearrayparser.h
 */

#ifndef RAWBYTEARRAYPARSER_H_
#define RAWBYTEARRAYPARSER_H_

#include <cstdint>
#include "byte_array.h"

class RawByteArrayParser
{
public:
	enum Endianness
	{
		BigEndian,
		LittleEndian
	};

	RawByteArrayParser(const void* buffer, int bufsize, Endianness endianness = LittleEndian);
	virtual ~RawByteArrayParser();

	uint8_t readByte();
	uint16_t readWord();
	uint32_t readDword();
	void readBytes(uint8_t* buffer, int count);
	byte_array readAll();

	void skip(int bytes);

	bool atEnd();

private:
	RawByteArrayParser(const RawByteArrayParser& parser) : m_endianness(LittleEndian) {}
	RawByteArrayParser& operator=(const RawByteArrayParser& other) { return *this; }
	const uint8_t* m_ptr;
	int m_left;
	Endianness m_endianness;
};

#endif /* RAWBYTEARRAYPARSER_H_ */
