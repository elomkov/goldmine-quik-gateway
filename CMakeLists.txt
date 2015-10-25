cmake_minimum_required(VERSION 3.0)
set(PROJECT goldmine-quik-gateway)
project(${PROJECT})

find_package(FLTK REQUIRED)

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

add_definitions(-DELPP_THREAD_SAFE -DELPP_STACKTRACE_ON_CRASH)

set(src
	log.cpp
	
	binary/byte_array.cpp
	binary/bytearraybuilder.cpp
	binary/bytearrayparser.cpp
	binary/rawbytearrayparser.cpp
	
	core/client.cpp
	core/dataimportserver.cpp
	core/datasink.cpp
	core/eventloop.cpp
	core/xlparser.cpp
	
	tables/currentparametertableparser.cpp
	
	time/timesource.cpp
	
	ui/mainwindow.cpp
	)

add_executable(${PROJECT} main.cpp ${src})
target_link_libraries(${PROJECT} ${FLTK_LIBRARIES})
target_link_libraries(${PROJECT} ${Boost_LIBRARIES} -lzmq)
