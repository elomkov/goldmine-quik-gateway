/*
 * client.cpp
 */

#include "client.h"
#include <boost/algorithm/string.hpp>
#include <map>

StreamMatcher::StreamMatcher(const std::string& ticker_, const std::initializer_list<goldmine::Datatype>& datatypes_) : enable(false)
{
	if(ticker_.size() < 2)
		throw std::runtime_error("Invalid ticker requested: " + ticker_ + " (too short, should be at least 2 characters)");

	if(ticker_[0] == '-')
		enable = false;
	else
		enable = true;

	auto tstart = ticker_.begin();
	auto tend = ticker_.end();
	if(ticker_[0] == '+')
	{
		tstart++;
	}
	ticker.assign(tstart, tend);

	if(enable)
	{
		if(datatypes_.size() == 0)
		{
			datatypes.insert(goldmine::Datatype::Price);
		}
		else
		{
			for(auto dt : datatypes_)
				datatypes.insert(dt);
		}
	}
}

StreamMatcher::StreamMatcher(const std::string& ticker_, const std::set<goldmine::Datatype>& datatypes_) : enable(true),
		ticker(ticker_), datatypes(datatypes_)
{

}

boost::tribool StreamMatcher::match(const std::string& ticker_, goldmine::Datatype datatype) const
{
	if(!enable)
		return false;

	if((ticker_ == ticker) && (datatypes.find(datatype) != datatypes.end()))
		return true;

	return boost::indeterminate;
}

Client::Client(const byte_array& peerId) : m_peerId(peerId)
{
}

Client::~Client()
{
}

void Client::addStreamMatcher(const std::string& selector)
{
	std::string streamId = selector;
	auto semicolon = selector.find_first_of(':');
	if(semicolon != std::string::npos)
	{
		std::string period = selector.substr(0, semicolon);
		if(period != "t")
			throw std::runtime_error("Unsupported period request: " + period + "; only tick requests are supported");
		streamId = selector.substr(semicolon + 1);
	}

	std::set<goldmine::Datatype> datatypes;
	auto slash = streamId.find_first_of('/');
	std::string ticker;
	if(slash == std::string::npos)
	{
		datatypes.insert(goldmine::Datatype::Price);
		ticker = streamId;
	}
	else
	{
		std::string rawDatatypes = streamId.substr(slash + 1);
		std::vector<std::string> datatypeIds;
		boost::split(datatypeIds, rawDatatypes, boost::is_any_of(","));

		for(const auto& datatypeId : datatypeIds)
		{
			datatypes.insert(mapDatatypeIdToDatatype(datatypeId));
		}
		ticker = streamId.substr(0, slash);
	}

	m_streamMatchers.push_back(StreamMatcher(ticker, datatypes));
}

goldmine::Datatype Client::mapDatatypeIdToDatatype(const std::string& datatypeId)
{
	std::map<std::string, goldmine::Datatype> map({
		{"price", goldmine::Datatype::Price},
		{"open_interest", goldmine::Datatype::OpenInterest},
		{"best_bid", goldmine::Datatype::BestBid},
		{"best_offer", goldmine::Datatype::BestOffer},
		{"depth", goldmine::Datatype::Depth},
		{"theory_price", goldmine::Datatype::TheoryPrice},
		{"volatility", goldmine::Datatype::Volatility},
		{"total_supply", goldmine::Datatype::TotalSupply},
		{"total_demand", goldmine::Datatype::TotalDemand}
	});

	auto it = map.find(datatypeId);
	if(it == map.end())
		throw std::runtime_error("Invalid datatype ID specified: " + datatypeId);

	return it->second;
}

bool Client::acceptsStream(const std::string& ticker, goldmine::Datatype datatype) const
{
	for(const auto& streamMatcher : m_streamMatchers)
	{
		auto matches = streamMatcher.match(ticker, datatype);
		if(matches)
			return true;
		else if(!matches)
			return false;
	}
	return false;
}
