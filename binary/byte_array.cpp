/*
 * byte_array.cpp
 *
 *  Created on: 15.09.2011
 *      Author: Denis Tereshkin <todin.dirihle@gmail.com>
 */

#include "byte_array.h"
#include <numeric>

std::ostream& operator<<(std::ostream& out, const byte_array& arr)
{
	out << std::hex << "byte_array(";
	for(byte_array::const_iterator it = arr.begin(); it != arr.end(); )
	{
		byte_array::const_iterator this_item = it++;
		if(it != arr.end())
		{
			out << (int)*this_item << ", ";
		}
		else
		{
			out << (int)*this_item;
		}
	}
	out << ")";
	return out;
}

byte_array operator+(const byte_array& arr1, const byte_array& arr2)
{
	byte_array a = arr1;
	a += arr2;
	return a;
}

byte_array& operator+=(byte_array& arr1, const byte_array& arr2)
{
	arr1.insert(arr1.end(), arr2.begin(), arr2.end());
	return arr1;
}

uint8_t checksum(const byte_array& arr)
{
	return std::accumulate(arr.begin(), arr.end(), 0) & 0xff;
}

byte_array byteArrayFromString(const std::string& str)
{
	byte_array arr;
	arr.assign(str.begin(), str.end());
	return arr;
}

byte_array byteArrayFromFixedString(const std::string& str, int length)
{
	auto arr = byteArrayFromString(str);
	arr.resize(length);
	return arr;
}
