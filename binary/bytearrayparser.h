#ifndef BYTEARRAYPARSER_H
#define BYTEARRAYPARSER_H

#include "byte_array.h"
#include <stdint.h>
#include <stdexcept>

class ByteArrayParser
{
public:
    enum Endianness
    {
        BigEndian,
        LittleEndian
    };

    ByteArrayParser(const byte_array& arr, Endianness endianness = LittleEndian);

    uint8_t readByte();
    uint16_t readWord();
    uint32_t readDword();
    byte_array readBytes(int count);
    byte_array readAll();

    void skip(int bytes);

    bool atEnd();

private:
    ByteArrayParser(const ByteArrayParser& parser) : m_endianness(LittleEndian) {}
    ByteArrayParser& operator=(const ByteArrayParser& other) { return *this; }
    byte_array m_array;
    byte_array::iterator m_it;
    Endianness m_endianness;
};

std::string parseFixedString(ByteArrayParser& parser, int length);

#endif // BYTEARRAYPARSER_H
