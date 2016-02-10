/*
 * trans2quik.h
 */

#ifndef CORE_TRANS2QUIK_TRANS2QUIK_H_
#define CORE_TRANS2QUIK_TRANS2QUIK_H_

#include <windows.h>
#include <string>

class Trans2QuikApi
{
public:
	typedef void (*TRANS2QUIK_CONNECTION_STATUS_CALLBACK)(long nConnectionEvent, long nExtendedErrorCode, LPSTR lpstrInfoMessage);
	typedef void (*TRANS2QUIK_TRANSACTION_REPLY_CALLBACK)(long nTransactionResult, long nTransactionExtendedErrorCode, long nTransactionReplyCode, DWORD dwTransId, double dOrderNum, LPSTR lpstrTransactionReplyMessage);
	typedef void (*TRANS2QUIK_ORDER_STATUS_CALLBACK)(long nMode, DWORD dwTransID, double dNumber, LPSTR lpstrClassCode, LPSTR lpstrSecCode, double dPrice, long nBalance, double dValue, long nIsSell, long nStatus, long nOrderDescriptor);
	typedef void (*TRANS2QUIK_TRADE_STATUS_CALLBACK)(long nMode, double dNumber, double dOrderNum, LPSTR lpstrClassCode, LPSTR lpstrSecCode, double dPrice, long nQty, double dValue, long nIsSell, long nTradeDescriptor);

	enum ReturnCodes
	{
		TRANS2QUIK_SUCCESS = 0,
		TRANS2QUIK_FAILED = 1,
		TRANS2QUIK_QUIK_TERMINAL_NOT_FOUND = 2,
		TRANS2QUIK_DLL_VERSION_NOT_SUPPORTED = 3,
		TRANS2QUIK_ALREADY_CONNECTED_TO_QUIK = 4,
		TRANS2QUIK_WRONG_SYNTAX = 5,
		TRANS2QUIK_QUIK_NOT_CONNECTED = 6,
		TRANS2QUIK_DLL_NOT_CONNECTED = 7,
		TRANS2QUIK_QUIK_CONNECTED = 8,
		TRANS2QUIK_QUIK_DISCONNECTED = 9,
		TRANS2QUIK_DLL_CONNECTED = 10,
		TRANS2QUIK_DLL_DISCONNECTED = 11,
		TRANS2QUIK_MEMORY_ALLOCATION_ERROR = 12,
		TRANS2QUIK_WRONG_CONNECTION_HANDLE = 13,
		TRANS2QUIK_WRONG_INPUT_PARAMS = 14
	};

	Trans2QuikApi(const std::wstring& dll);
	virtual ~Trans2QuikApi();

	long (*TRANS2QUIK_CONNECT)(LPCSTR lpcstrConnectionParamsString, long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize);
	long (*TRANS2QUIK_DISCONNECT)(long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize);
	long (*TRANS2QUIK_IS_QUIK_CONNECTED)(long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize);
	long (*TRANS2QUIK_IS_DLL_CONNECTED)(long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize);
	long (*TRANS2QUIK_SEND_SYNC_TRANSACTION)(LPSTR lpstTransactionString, long* pnReplyCode, PDWORD pdwTransId, double* pdOrderNum, LPSTR lpstrResultMessage, DWORD dwResultMessageSize, long* pnExtendedErrorCode, LPSTR lpstErrorMessage, DWORD dwErrorMessageSize);
	long (*TRANS2QUIK_SEND_ASYNC_TRANSACTION)(LPSTR lpstTransactionString, long* pnExtendedErrorCode, LPSTR lpstErrorMessage, DWORD dwErrorMessageSize);
	long (*TRANS2QUIK_SET_CONNECTION_STATUS_CALLBACK)(TRANS2QUIK_CONNECTION_STATUS_CALLBACK pfConnectionStatusCallback, long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize);
	long (*TRANS2QUIK_SET_TRANSACTIONS_REPLY_CALLBACK)(TRANS2QUIK_TRANSACTION_REPLY_CALLBACK pfTransactionReplyCallback, long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize);
	long (*TRANS2QUIK_SUBSCRIBE_ORDERS)(LPSTR lpstrClassCode, LPSTR lpstrSeccodes);
	long (*TRANS2QUIK_SUBSCRIBE_TRADES)(LPSTR lpstrClassCode, LPSTR lpstrSeccodes);
	void (*TRANS2QUIK_START_ORDERS)(TRANS2QUIK_ORDER_STATUS_CALLBACK pfnOrderStatusCallback);
	void (*TRANS2QUIK_START_TRADES)(TRANS2QUIK_TRADE_STATUS_CALLBACK pfnTradesStatusCallback);
	long (*TRANS2QUIK_UNSUBSCRIBE_ORDERS)();
	long (*TRANS2QUIK_UNSUBSCRIBE_TRADES)();

private:
	HANDLE m_handle;
};

#endif /* CORE_TRANS2QUIK_TRANS2QUIK_H_ */
