
#include "quikbroker.h"
#include "exceptions.h"
#include <stdexcept>
#include <vector>
#include <windows.h>

#include "log.h"

static int gs_id = 0;

using namespace boost;

time_t _mkgmtime(const struct tm *tm) 
{
	// Month-to-day offset for non-leap-years.
	static const int month_day[12] =
	{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

	// Most of the calculation is easy; leap years are the main difficulty.
	int month = tm->tm_mon % 12;
	int year = tm->tm_year + tm->tm_mon / 12;
	if (month < 0) {   // Negative values % 12 are still negative.
		month += 12;
		--year;
	}

	// This is the number of Februaries since 1900.
	const int year_for_leap = (month > 1) ? year + 1 : year;

	time_t rt = tm->tm_sec                             // Seconds
		+ 60 * (tm->tm_min                          // Minute = 60 seconds
				+ 60 * (tm->tm_hour                         // Hour = 60 minutes
					+ 24 * (month_day[month] + tm->tm_mday - 1  // Day = 24 hours
						+ 365 * (year - 70)                         // Year = 365 days
						+ (year_for_leap - 69) / 4                  // Every 4 years is     leap...
						- (year_for_leap - 1) / 100                 // Except centuries...
						+ (year_for_leap + 299) / 400)));           // Except 400s.
	return rt < 0 ? -1 : rt;
}

std::pair<std::string, std::string> splitStringByTwo(const std::string& str, char c)
{
	auto charPosition = str.find(c);
	if(charPosition == std::string::npos)
		BOOST_THROW_EXCEPTION(ParameterError());

	auto first = str.substr(0, charPosition);
	auto second = str.substr(charPosition + 1, std::string::npos);

	return std::make_pair(first, second);
}

static std::wstring utf8toUtf16(const std::string & str)
{
	if (str.empty())
		return std::wstring();

	size_t charsNeeded = ::MultiByteToWideChar(CP_UTF8, 0,
			str.data(), (int)str.size(), NULL, 0);
	if (charsNeeded == 0)
		throw std::runtime_error("Failed converting UTF-8 string to UTF-16");

	std::vector<wchar_t> buffer(charsNeeded);
	int charsConverted = ::MultiByteToWideChar(CP_UTF8, 0,
			str.data(), (int)str.size(), &buffer[0], buffer.size());
	if (charsConverted == 0)
		throw std::runtime_error("Failed converting UTF-8 string to UTF-16");

	return std::wstring(&buffer[0], charsConverted);
}

QuikBroker::QuikBroker(const std::map<std::string, std::string>& config)
{
	auto quikDllPath = config.at("dll_path");
	m_quik = std::make_unique<Trans2QuikApi>(utf8toUtf16(quikDllPath));
	auto quikPath = config.at("quik_path");
	long extendedErrorCode;
	std::array<char, 512> buffer;
	long rc = m_quik->TRANS2QUIK_CONNECT(quikPath.c_str(), &extendedErrorCode, buffer.data(), buffer.size());
	if(rc != Trans2QuikApi::TRANS2QUIK_SUCCESS)
		BOOST_THROW_EXCEPTION(ExternalApiError() << errinfo_str("Error while connecting to quik"));

	assert(!m_instance);
	m_instance = this;
}

QuikBroker::~QuikBroker()
{
	m_instance = nullptr;
}

void QuikBroker::submitOrder(const Order::Ptr& order)
{
	boost::unique_lock<boost::recursive_mutex> lock(m_mutex);
	LOG(DEBUG) << "Order: " << order->stringRepresentation();
	int transactionId = gs_id++;
	m_unsubmittedOrders[transactionId] = order;

	std::string transactionString = makeTransactionStringForOrder(order, transactionId);

	long extendedErrorCode;
	std::array<char, 512> buffer;
	long rc = m_quik->TRANS2QUIK_SEND_ASYNC_TRANSACTION(transactionString.c_str(), &extendedErrorCode, buffer.data(), buffer.size());
	if(rc != Trans2QuikApi::TRANS2QUIK_SUCCESS)
		BOOST_THROW_EXCEPTION(ExternalApiError() << errinfo_str(std::string("Unable to send transaction: ") + buffer.data()));
}

void QuikBroker::cancelOrder(const Order::Ptr& order)
{
	boost::unique_lock<boost::recursive_mutex> lock(m_mutex);
}

void QuikBroker::registerOrderCallback(const OrderCallback& callback)
{
	boost::unique_lock<boost::recursive_mutex> lock(m_mutex);
	m_orderCallbacks.push_back(callback);
}

void QuikBroker::unregisterOrderCallback(const OrderCallback& callback)
{
}

void QuikBroker::registerTradeCallback(const TradeCallback& callback)
{
	boost::unique_lock<boost::recursive_mutex> lock(m_mutex);
	m_tradeCallbacks.push_back(callback);
}

Order::Ptr QuikBroker::order(int localId)
{
	boost::unique_lock<boost::recursive_mutex> lock(m_mutex);
}

std::list<std::string> QuikBroker::accounts()
{
}

std::list<Position> QuikBroker::positions()
{
	boost::unique_lock<boost::recursive_mutex> lock(m_mutex);
}


void QuikBroker::connectionStatusCallback(long event, long errorCode, LPSTR infoMessage)
{
	boost::unique_lock<boost::recursive_mutex> lock(m_instance->m_mutex);
	long rc;
	long extendedErrorCode;
	std::array<char, 512> buffer;

	if(event == Trans2QuikApi::TRANS2QUIK_DLL_CONNECTED)
	{
		rc = m_quik->TRANS2QUIK_SET_CONNECTION_STATUS_CALLBACK(&QuikBroker::connectionStatusCallback, &extendedErrorCode, buffer.data(), buffer.size());
		if(rc != Trans2QuikApi::TRANS2QUIK_SUCCESS)
			BOOST_THROW_EXCEPTION(ExternalApiError() << errinfo_str("Unable to set connection status callback"));

		rc = m_quik->TRANS2QUIK_SET_TRANSACTIONS_REPLY_CALLBACK(&QuikBroker::transactionReplyCallback, &extendedErrorCode, buffer.data(), buffer.size());
		if(rc != Trans2QuikApi::TRANS2QUIK_SUCCESS)
			BOOST_THROW_EXCEPTION(ExternalApiError() << errinfo_str("Unable to set transaction reply callback"));

		rc = m_quik->TRANS2QUIK_SUBSCRIBE_ORDERS("", "");
		if(rc != Trans2QuikApi::TRANS2QUIK_SUCCESS)
			BOOST_THROW_EXCEPTION(ExternalApiError() << errinfo_str("Unable to subscribe to orders"));

		rc = m_quik->TRANS2QUIK_START_ORDERS(&QuikBroker::orderStatusCallback);
		if(rc != Trans2QuikApi::TRANS2QUIK_SUCCESS)
			BOOST_THROW_EXCEPTION(ExternalApiError() << errinfo_str("Unable to start orders stream"));

		rc = m_quik->TRANS2QUIK_SUBSCRIBE_TRADES("", "");
		if(rc != Trans2QuikApi::TRANS2QUIK_SUCCESS)
			BOOST_THROW_EXCEPTION(ExternalApiError() << errinfo_str("Unable to subscribe to trades"));

		rc = m_quik->TRANS2QUIK_START_TRADES(&QuikBroker::tradeCallback);
		if(rc != Trans2QuikApi::TRANS2QUIK_SUCCESS)
			BOOST_THROW_EXCEPTION(ExternalApiError() << errinfo_str("Unable to start trades stream"));
	}
}

void QuikBroker::transactionReplyCallback(long result, long errorCode, long transactionReplyCode, DWORD transactionId, double orderNum, LPSTR transactionReplyMessage)
{
	boost::unique_lock<boost::recursive_mutex> lock(m_instance->m_mutex);
	LOG(DEBUG) << "Transaction reply: " << transactionId << "; result: " << result;

	auto orderIt = m_unsubmittedOrders.find(transactionId);
	if(orderIt == m_unsubmittedOrders.end())
	{
		LOG(WARNING) << "Reply with unknown transactionID";
		return;
	}
	auto order = *orderIt;
	m_unsubmittedOrders.erase(orderIt);

	if(result == Trans2QuikApi::TRANS2QUIK_SUCCESS)
	{
		m_pendingOrders[orderNum] = order;
		m_orderIdMap[orderNum] = order->clientAssignedId();
		order->updateState(Order::State::Submitted);
	}
	else
	{
		m_retiredOrders.push_back(order);
		order->updateState(Order::State::Rejected);
	}

	executeOrderStateCallbacks(order);
}

void QuikBroker::orderStatusCallback(long mode, DWORD transactionId, double number,
		LPSTR classCode, LPSTR secCode, double price, long balance, double volume, long isBuy, long status, long orderDescriptor)
{
	boost::unique_lock<boost::recursive_mutex> lock(m_instance->m_mutex);
	LOG(DEBUG) << "Order status: " << number << "; status: " << status;
	// TODO handle other modes
	if(mode == 0)
	{
		auto orderIt = m_pendingOrders.find(number);
		if(orderIt == m_pendingOrders.end())
		{
			LOG(WARNING) << "Incoming order status for unknown order";
			return;
		}

		auto order = *orderIt;
		if((status != 1) && (status != 2)) // Order executed
		{
			if(balance == 0) // Order fully filled
			{
				order->updateState(Order::State::Executed);
				m_pendingOrders.erase(orderIt);
				m_retiredOrders.push_back(orderIt);
			}
			// TODO partial execution
		}
		else if(status == 2)
		{
			order->updateState(Order::State::Cancelled);
			m_pendingOrders.erase(orderIt);
			m_retiredOrders.push_back(orderIt);
		}
		else if(status == 1)
		{
			order->updateState(Order::State::Submitted);
		}
		executeOrderStateCallbacks(order);
	}
}

void QuikBroker::tradeCallback(long mode, double number, double orderNumber, LPSTR classCode, double price, long quantity, double volume, long isSell, long tradeDescriptor)
{
	boost::unique_lock<boost::recursive_mutex> lock(m_instance->m_mutex);
	if(mode == 0)
	{
		auto it = m_pendingOrders.find(orderNumber);
		if(it == m_pendingOrders.end())
		{
			LOG(WARNING) << "Incoming trade for unknown order: " << orderNumber;
			return;
		}
		auto order = *it;
		Trade trade;
		trade.orderId = *it;
		trade.amount = quantity;
		trade.operation = (isSell ? Order::Operation::Sell : Order::Operation::Buy);
		trade.account = order->account();
		trade.ticker = order->security();

		long ymd = m_quik->TRANS2QUIK_TRADE_DATE_TIME(tradeDescriptor, 0);
		long hms = m_quik->TRANS2QUIK_TRADE_DATE_TIME(tradeDescriptor, 1);
		long us = m_quik->TRANS2QUIK_TRADE_DATE_TIME(tradeDescriptor, 2);
		struct tm tm;
		tm.tm_year = ymd / 10000 - 1900;
		tm.tm_mon = (ymd % 10000) / 100 - 1;
		tm.tm_mday = (ymd % 100);
		tm.tm_hour = hms / 10000;
		tm.tm_min = (hms % 10000) / 100;
		tm.tm_sec = (hms % 100);
		trade.timestamp = _mkgmtime(&tm);
		trade.useconds = us;

		executeTradeCallback(trade);
	}
}

std::string QuikBroker::makeTransactionStringForOrder(const Order::Ptr& order, int transactionId)
{
	std::string account, clientCode;
	try
	{
		std::tie(account, clientCode) = splitStringByTwo(order->account(), '#');
	}
	catch(ParameterError& e)
	{
		e << errinfo("Invalid account ID, should be of the following form: ACCOUNT#CLIENT_CODE");
		throw e;
	}

	std::string classCode, secCode;
	try
	{
		std::tie(classCode, secCode) = splitStringByTwo(order->security(), '#');
	}
	catch(ParameterError& e)
	{
		e << errinfo("Invalid security ID, should be of the following form: CLASSCODE#SECCODE");
		throw e;
	}

	if((order->type() == Order::Type::Market) || (order->type() == Order::Type::Limit))
	{
		std::array<char, 1024> buf;
		char orderTypeCode = (order->type() == Order::Type::Market ? 'M' : 'L');
		char operationCode = (order->operation() == Order::Operation::Buy ? 'B' : 'S');
		int stringSize = snprintf(buf.data(), 1024, "ACCOUNT=%s;CLIENT_CODE=%s;TYPE=%c;TRANS_ID=%d;CLASSCODE=%s;SECCODE=%s;ACTION=NEW_CODE;OPERATION=%c;PRICE=%f;QUANTITY=%d;",
				account.c_str(), clientCode.c_str(), orderTypeCode, transactionId, classCode.c_str(), secCode.c_str(), operationCode, order->price(), order->amount());
		return std::string(buf.data(), stringSize + 1);
	}
}

void QuikBroker::executeOrderStateCallbacks(const Order::Ptr& order)
{
	for(const auto& callback : m_orderCallbacks)
	{
		callback(order);
	}
}

void QuikBroker::executeTradeCallback(const Trade& trade)
{
	for(const auto& callback : m_tradeCallbacks)
	{
		callback(trade);
	}
}
