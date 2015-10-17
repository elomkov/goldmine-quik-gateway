#include "bytearraybuilder.h"

static uint16_t s_htons(uint16_t v)
{
	return ((v & 0xff) << 8) | ((v >> 8) & 0xff);
}

static uint32_t s_htonl(uint32_t v)
{
	return ((v & 0xff) << 24) | ((v & 0xff00) << 8) |
			((v >> 24) & 0xff) | ((v >> 8) & 0xff00);
}

ByteArrayBuilder::ByteArrayBuilder(Endianness endianness) : m_endianness(endianness)
{
}

void ByteArrayBuilder::appendByte(uint8_t value)
{
    m_array.push_back(value);
}

void ByteArrayBuilder::appendWord(uint16_t value)
{
	uint16_t v = (m_endianness == BigEndian) ? value : s_htons(value);
    m_array.push_back((v >> 8) & 0xff);
    m_array.push_back(v & 0xff);
}

void ByteArrayBuilder::appendDword(uint32_t value)
{
	uint32_t v = (m_endianness == BigEndian) ? value : s_htonl(value);
    m_array.push_back((v >> 24) & 0xff);
    m_array.push_back((v >> 16) & 0xff);
    m_array.push_back((v >> 8) & 0xff);
    m_array.push_back(v & 0xff);
}

void ByteArrayBuilder::appendBytes(const byte_array& bytes)
{
	std::copy(bytes.begin(), bytes.end(), std::back_inserter(m_array));
}

byte_array ByteArrayBuilder::getArray() const
{
    return m_array;
}

ByteArrayBuilder& ByteArrayBuilder::byte(uint8_t value)
{
	appendByte(value);
	return *this;
}

ByteArrayBuilder& ByteArrayBuilder::word(uint16_t value)
{
	appendWord(value);
	return *this;
}

ByteArrayBuilder& ByteArrayBuilder::dword(uint32_t value)
{
	appendDword(value);
	return *this;
}

ByteArrayBuilder& ByteArrayBuilder::bytes(const byte_array& value)
{
	appendBytes(value);
	return *this;
}

void ByteArrayBuilder::clear()
{
	m_array.clear();
}

ByteArrayBuilder rawData()
{
	return ByteArrayBuilder();
}
