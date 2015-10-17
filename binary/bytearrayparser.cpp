#include "bytearrayparser.h"

static uint16_t s_htons(uint16_t v)
{
	return ((v & 0xff) << 8) | ((v >> 8) & 0xff);
}

static uint32_t s_htonl(uint32_t v)
{
	return ((v & 0xff) << 24) | ((v & 0xff00) << 8) |
			((v >> 24) & 0xff) | ((v >> 8) & 0xff00);
}

ByteArrayParser::ByteArrayParser(const byte_array& arr, Endianness endianness) : m_array(arr), m_endianness(endianness)
{
	m_it = m_array.begin();
}

uint8_t ByteArrayParser::readByte()
{
	if(atEnd())
		throw StreamEndException();
    return *m_it++;
}

uint16_t ByteArrayParser::readWord()
{
	if(std::distance(m_it, m_array.end()) < 2)
		throw StreamEndException();
    uint8_t b[2];
    b[0] = *m_it++;
    b[1] = *m_it++;

    uint16_t result = b[1] | ((uint32_t)b[0] << 8);
    if(m_endianness == LittleEndian)
        result = s_htons(result);
    return result;
}

uint32_t ByteArrayParser::readDword()
{
	if(std::distance(m_it, m_array.end()) < 4)
		throw StreamEndException();
    uint8_t b[4];
    b[0] = *m_it++;
    b[1] = *m_it++;
    b[2] = *m_it++;
    b[3] = *m_it++;

    uint32_t result = b[3] | (((uint32_t)b[2]) << 8) | (((uint32_t)b[1]) << 16) | (((uint32_t)b[0]) << 24);
    if(m_endianness == LittleEndian)
        result = s_htonl(result);
    return result;
}

byte_array ByteArrayParser::readBytes(int count)
{
	if(std::distance(m_it, m_array.end()) < count)
		throw StreamEndException();
	byte_array arr;
    std::copy(m_it, m_it + count, std::back_inserter(arr));
    m_it += count;
    return arr;
}

byte_array ByteArrayParser::readAll()
{
    byte_array arr;
    std::copy(m_it, m_array.end(), std::back_inserter(arr));
    return arr;
}

void ByteArrayParser::skip(int bytes)
{
	if(std::distance(m_it, m_array.end()) < bytes)
		throw StreamEndException();
	m_it += bytes;
}

bool ByteArrayParser::atEnd()
{
    return m_it == m_array.end();
}

std::string parseFixedString(ByteArrayParser& parser, int length)
{
	return std::string(reinterpret_cast<const char*>(parser.readBytes(length).data()));
}
