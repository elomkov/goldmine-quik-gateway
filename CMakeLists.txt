cmake_minimum_required(VERSION 3.0)
set(PROJECT goldmine-quik-gateway)
project(${PROJECT})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/gtest)

set(CMAKE_CXX_FLAGS "-std=gnu++14")

# boost
set (BOOST_ROOT ${CMAKE_PREFIX_PATH})
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_MULTITHREADED ON)

add_definitions(-DBOOST_THREAD_USE_LIB)

find_package(Boost COMPONENTS system thread REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(3rdparty/jsoncpp/json)

add_definitions(-DELPP_THREAD_SAFE -DELPP_STACKTRACE_ON_CRASH)

set(src
	3rdparty/jsoncpp/jsoncpp.cpp

	log.cpp
	
	binary/byte_array.cpp
	binary/bytearraybuilder.cpp
	binary/bytearrayparser.cpp
	binary/rawbytearrayparser.cpp
	
	core/client.cpp
	core/dataimportserver.cpp
	core/datasink.cpp
	core/eventloop.cpp
	
	core/xltable/xlparser.cpp
	core/xltable/xltable.cpp
	
	tables/alldealstableparser.cpp
	tables/currentparametertableparser.cpp
	
	time/timesource.cpp
	
	ui/mainwindow.cpp
	)

add_executable(${PROJECT} main.cpp ${src})
target_link_libraries(${PROJECT} -lfltk)
target_link_libraries(${PROJECT} ${Boost_LIBRARIES} -lzmq)

# testing

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/Catch
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/Catch/single_include
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/fused_gmock

	${CMAKE_CURRENT_BINARY_DIR}
	)
	
set(test_sources
	3rdparty/fused_gmock/gmock-gtest-all.cc
	tests/test.cpp
	
	tests/xl_test.cpp
	)
	
add_executable(${PROJECT}-test ${test_sources} ${src})
set_target_properties(${PROJECT}-test PROPERTIES COMPILE_FLAGS "-O0 -g -fprofile-arcs -ftest-coverage")
set_target_properties(${PROJECT}-test PROPERTIES LINK_FLAGS "-fprofile-arcs -lgcov")
target_link_libraries(${PROJECT}-test  ${Boost_LIBRARIES} -lfltk -lzmq)

if(UNIX)
	target_link_libraries(${PROJECT}-test ${Boost_LIBRARIES} -lpthread -ldl)
endif(UNIX)
