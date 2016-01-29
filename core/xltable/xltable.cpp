/*
 * xltable.cpp
 */

#include "xltable.h"

XlTable::XlTable() : m_width(0), m_height(0)
{
}

XlTable::XlTable(int width, int height) : m_width(width), m_height(height)
{
	m_data.resize(width * height, XlCell(XlEmpty()));
}

XlTable::~XlTable()
{
}

int XlTable::width() const
{
	return m_width;
}

int XlTable::height() const
{
	return m_height;
}

void XlTable::set(int row, int column, const XlCell& value)
{
	m_data[row * m_width + column] = value;
}

XlTable::XlCell XlTable::get(int row, int column)
{
	return m_data[row * m_width + column];
}
