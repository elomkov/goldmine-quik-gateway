/*
 * xlparser.h
 */

#ifndef CORE_XLPARSER_H_
#define CORE_XLPARSER_H_

#include <boost/variant.hpp>
#include "binary/rawbytearrayparser.h"
#include "xltable.h"

struct XlPosition
{
	XlPosition(int row_, int column_, int width_, int height_): row(row_), column(column_),
			width(width_), height(height_) {}

	void incrementPosition()
	{
		column++;
		if(column >= width)
		{
			column = 0;
			row++;
		}

		// FIXME: What if row overflows?
	}

	size_t linearize() const
	{
		return row * width + column;
	}

	int row;
	int column;
	int width;
	int height;
};


class XlParser
{
public:
	XlParser();
	virtual ~XlParser();

	void parse(uint8_t* data, int datalength);

	XlTable::Ptr getParsedTable() const { return m_table; }

private:
	int parseString(RawByteArrayParser& parser, XlPosition& pos);
	int parseFloat(RawByteArrayParser& parser, XlPosition& pos);

private:
	XlTable::Ptr m_table;
};

#endif /* CORE_XLPARSER_H_ */
