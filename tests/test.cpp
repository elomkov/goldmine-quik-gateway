/*
 * test.cpp
 */

#define CATCH_CONFIG_RUNNER

#include "catch.hpp"
#include "gmock/gmock.h"

int main(int argc, char** argv)
{
	::testing::GTEST_FLAG(throw_on_failure) = true;
	::testing::InitGoogleMock(&argc, argv);

	int result = Catch::Session().run( argc, argv );
	return result;
}

