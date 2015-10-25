/*
 * client.h
 */

#ifndef CORE_CLIENT_H_
#define CORE_CLIENT_H_

#include "binary/byte_array.h"
#include <memory>
#include <set>
#include <boost/logic/tribool.hpp>
#include "goldmine/data.h"

struct StreamMatcher
{
	bool enable;
	std::string ticker;
	std::set<goldmine::Datatype> datatypes;

	StreamMatcher(const std::string& ticker, const std::initializer_list<goldmine::Datatype>& datatypes = std::initializer_list<goldmine::Datatype>());
	StreamMatcher(const std::string& ticker, const std::set<goldmine::Datatype>& datatypes);

	boost::tribool match(const std::string& ticker, goldmine::Datatype datatype) const;
};

class Client
{
public:
	typedef std::shared_ptr<Client> Ptr;

	Client(const byte_array& peerId);
	virtual ~Client();

	byte_array peerId() const { return m_peerId; }

	void addStreamMatcher(const std::string& selector);

	bool acceptsStream(const std::string& ticker, goldmine::Datatype datatype) const;

private:
	goldmine::Datatype mapDatatypeIdToDatatype(const std::string& datatypeId);

private:
	byte_array m_peerId;
	std::vector<StreamMatcher> m_streamMatchers;
};

#endif /* CORE_CLIENT_H_ */
