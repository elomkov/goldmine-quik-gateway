#ifndef BYTEARRAYBUILDER_H
#define BYTEARRAYBUILDER_H

#include "byte_array.h"
#include <stdint.h>

class ByteArrayBuilder
{
public:
    enum Endianness
    {
        BigEndian,
        LittleEndian
    };

    ByteArrayBuilder(Endianness endianness = LittleEndian);

    void appendByte(uint8_t value);
    void appendWord(uint16_t value);
    void appendDword(uint32_t value);
    void appendBytes(const byte_array& bytes);

    ByteArrayBuilder& byte(uint8_t value);
    ByteArrayBuilder& word(uint16_t value);
    ByteArrayBuilder& dword(uint32_t value);
    ByteArrayBuilder& bytes(const byte_array& value);

    byte_array getArray() const;

    void clear();

private:
    byte_array m_array;
    Endianness m_endianness;
};

ByteArrayBuilder rawData();

#endif // BYTEARRAYBUILDER_H
