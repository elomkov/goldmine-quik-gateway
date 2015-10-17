/*
 * rawbytearrayparser.cpp
 */

#include "rawbytearrayparser.h"

static uint16_t s_htons(uint16_t v)
{
	return ((v & 0xff) << 8) | ((v >> 8) & 0xff);
}

static uint32_t s_htonl(uint32_t v)
{
	return ((v & 0xff) << 24) | ((v & 0xff00) << 8) |
			((v >> 24) & 0xff) | ((v >> 8) & 0xff00);
}


RawByteArrayParser::RawByteArrayParser(const void* buffer, int bufsize, Endianness endianness) :
	m_ptr((const uint8_t*)buffer),
	m_left(bufsize),
	m_endianness(endianness)
{
}

RawByteArrayParser::~RawByteArrayParser()
{
}

uint8_t RawByteArrayParser::readByte()
{
	if(atEnd())
		throw StreamEndException();
	m_left--;
	return *m_ptr++;
}

uint16_t RawByteArrayParser::readWord()
{
	if(m_left < 2)
		throw StreamEndException();
	m_left -= 2;

	uint8_t b[2];
	b[0] = *m_ptr++;
	b[1] = *m_ptr++;

	uint16_t result = b[1] | ((uint32_t)b[0] << 8);
	if(m_endianness == LittleEndian)
		result = s_htons(result);
	return result;
}

uint32_t RawByteArrayParser::readDword()
{
	if(m_left < 4)
		throw StreamEndException();
	uint8_t b[4];
	b[0] = *m_ptr++;
	b[1] = *m_ptr++;
	b[2] = *m_ptr++;
	b[3] = *m_ptr++;

	uint32_t result = b[3] | (((uint32_t)b[2]) << 8) | (((uint32_t)b[1]) << 16) | (((uint32_t)b[0]) << 24);
	if(m_endianness == LittleEndian)
		result = s_htonl(result);
	return result;
}

void RawByteArrayParser::readBytes(uint8_t* buffer, int count)
{
	if(m_left < count)
		throw StreamEndException();
	memcpy(buffer, m_ptr, count);
	m_left -= count;
	m_ptr += count;
}

byte_array RawByteArrayParser::readAll()
{
	byte_array b(m_ptr, m_ptr + m_left);
	m_ptr += m_left;
	m_left = 0;
	return b;
}

void RawByteArrayParser::skip(int bytes)
{
	if(m_left < bytes)
		throw StreamEndException();
	m_ptr += bytes;
	m_left -= bytes;
}

bool RawByteArrayParser::atEnd()
{
	return m_left == 0;
}
