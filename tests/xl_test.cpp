/*
 * xl_test.cpp
 */

#include "catch.hpp"
#include "core/xltable/xltable.h"

TEST_CASE("XlTable", "[xl][xl_table]")
{
	SECTION("Empty XlTable has zero dimensions")
	{
		XlTable xl;

		REQUIRE(xl.width() == 0);
		REQUIRE(xl.height() == 0);
	}

	SECTION("Table setters and getters")
	{
		XlTable xl(10, 5);

		XlTable::XlEmpty empty = boost::get<XlTable::XlEmpty>(xl.get(3, 7));

		xl.set(3, 7, 1.0);

		double val = boost::get<double>(xl.get(3, 7));

		REQUIRE(val);

		REQUIRE(val == 1.0);
	}
}


