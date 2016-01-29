/*
 * xlparser.cpp
 */

#include "xlparser.h"
#include <boost/format.hpp>
#include <array>
#include "log.h"

using namespace boost;

enum BlockType
{
	tdtTable = 16,
	tdtFloat = 1,
	tdtString = 2,
	tdtBool = 3,
	tdtError = 4,
	tdtBlank = 5,
	tdtInt = 6,
	tdtSkip = 7
};

XlParser::XlParser()
{
}

XlParser::~XlParser()
{
}

void XlParser::parse(uint8_t* data, int datalength)
{
	RawByteArrayParser parser(data, datalength);

	int datatype = parser.readWord();
	if(datatype != tdtTable)
		throw std::runtime_error("First entry is not Table");

	int blocksize = parser.readWord();
	if(blocksize != 4)
		throw std::runtime_error("Invalid table entry size: should be 4, got " + std::to_string(blocksize));

	int height = parser.readWord();
	int width = parser.readWord();

	m_table = std::make_shared<XlTable>(width, height);

	LOG(DEBUG) << "Table size: " << width << "x" << height;

	XlPosition pos(0, 0, width, height);

	while(!parser.atEnd())
	{
		datatype = parser.readWord();
		blocksize = parser.readWord();

		switch(datatype)
		{
		case tdtString:
			while(blocksize > 0)
			{
				blocksize -= parseString(parser, pos);
				pos.incrementPosition();
			}
			break;
		case tdtFloat:
			while(blocksize > 0)
			{
				blocksize -= parseFloat(parser, pos);
				pos.incrementPosition();
			}
			break;
		case tdtBlank:
			{
				int fields = parser.readWord();
				while(fields-- > 0)
					pos.incrementPosition();
			}
			break;
		default:
			LOG(WARNING) << "Unparsed field: " << datatype;
			break;
		}
	}
}

int XlParser::parseString(RawByteArrayParser& parser, XlPosition& pos)
{
	int length = parser.readByte();
	if(length == 0)
	{
		return 1;
	}
	std::vector<uint8_t> buf(length);
	parser.readBytes(buf.data(), length);
	std::string s(buf.begin(), buf.end());
	m_table->set(pos.row, pos.column, s);
	return length + 1;
}

int XlParser::parseFloat(RawByteArrayParser& parser, XlPosition& pos)
{
	double value = 0;
	parser.readBytes(reinterpret_cast<uint8_t*>(&value), 8);
	m_table->set(pos.row, pos.column, value);
	return 8;
}
