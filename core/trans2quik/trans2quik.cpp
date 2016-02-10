/*
 * trans2quik.cpp
 */

#include "trans2quik.h"

#define LOAD_SYMBOL(sym) sym = GetProcAddress(m_handle, #sym); if(!sym) throw std::runtime_error("Unable to obtain symbol: " #sym);

Trans2QuikApi::Trans2QuikApi(const std::wstring& dll)
{
	m_handle = GetModuleHandleW(dll.c_str());

	LOAD_SYMBOL(TRANS2QUIK_CONNECT);
	LOAD_SYMBOL(TRANS2QUIK_DISCONNECT);
	LOAD_SYMBOL(TRANS2QUIK_IS_QUIK_CONNECTED);
	LOAD_SYMBOL(TRANS2QUIK_IS_DLL_CONNECTED);
	LOAD_SYMBOL(TRANS2QUIK_SEND_SYNC_TRANSACTION);
	LOAD_SYMBOL(TRANS2QUIK_SEND_ASYNC_TRANSACTION);
	LOAD_SYMBOL(TRANS2QUIK_SET_CONNECTION_STATUS_CALLBACK);
	LOAD_SYMBOL(TRANS2QUIK_SET_TRANSACTIONS_REPLY_CALLBACK);
	LOAD_SYMBOL(TRANS2QUIK_SUBSCRIBE_ORDERS);
	LOAD_SYMBOL(TRANS2QUIK_SUBSCRIBE_TRADES);
	LOAD_SYMBOL(TRANS2QUIK_START_ORDERS);
	LOAD_SYMBOL(TRANS2QUIK_START_TRADES);
	LOAD_SYMBOL(TRANS2QUIK_UNSUBSCRIBE_ORDERS);
	LOAD_SYMBOL(TRANS2QUIK_UNSUBSCRIBE_TRADES);
}

Trans2QuikApi::~Trans2QuikApi()
{
	CloseHandle(m_handle);
}
4