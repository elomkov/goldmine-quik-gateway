/*
 * xltable.h
 */

#ifndef CORE_XLTABLE_XLTABLE_H_
#define CORE_XLTABLE_XLTABLE_H_

#include <memory>
#include <boost/variant.hpp>

class XlTable
{
public:
	typedef std::shared_ptr<XlTable> Ptr;

	struct XlEmpty {};
	typedef boost::variant<int, double, std::string, XlEmpty> XlCell;

	XlTable();
	XlTable(int width, int height);
	virtual ~XlTable();

	int width() const;
	int height() const;

	void set(int row, int column, const XlCell& value);
	XlCell get(int row, int column);

private:
	int m_width;
	int m_height;

	std::vector<XlCell> m_data;
};

#endif /* CORE_XLTABLE_XLTABLE_H_ */
