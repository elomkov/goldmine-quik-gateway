/*
 * quotetable.h
 */

#ifndef CORE_QUOTETABLE_H_
#define CORE_QUOTETABLE_H_

#include "goldmine/data.h"
#include <utility>
#include <unordered_map>
#include <memory>


class QuoteTable
{
public:
	typedef std::shared_ptr<QuoteTable> Ptr;

	QuoteTable();
	virtual ~QuoteTable();

	void updateQuote(const std::string& ticker, const goldmine::Tick& tick);
	goldmine::Tick lastQuote(const std::string& ticker, goldmine::Datatype datatype);

private:
	typedef std::pair<std::string, goldmine::Datatype> Key;

	class KeyHasher
	{
	public:
		std::size_t operator()(const Key& k) const
		{
			return std::hash<std::string>()(k.first) ^ (std::hash<int>()((int)k.second) << 1);
		}
	};

	std::unordered_map<Key, goldmine::Tick, KeyHasher> m_table;
};

#endif /* CORE_QUOTETABLE_H_ */
