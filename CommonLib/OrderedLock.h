/**
 * @file OrderedLock.h
 * @brief OrderLock作成クラス宣言
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */

#include <Windows.h>
#include <process.h>
#include <memory>
#include <exception>
#include "../CommonLib/MemoryLoan.h"
#include "../Debug_fnc/debug_fnc.h"
#pragma comment(lib,  "../Debug_fnc/" STRINGIZE($CONFIGURATION) "/Debug_fnc-" STRINGIZE($CONFIGURATION) ".lib")

#pragma once
class OrderedLock{
	static constexpr DWORD NUM_LOCKS = 0x4000;
public:
	OrderedLock();
	OrderedLock(const OrderedLock&) = delete;
	OrderedLock& operator=(const OrderedLock&) = delete;
	OrderedLock& operator()(const OrderedLock&) = delete;
	OrderedLock(OrderedLock&&)noexcept = delete;
	OrderedLock& operator=(OrderedLock&&)noexcept = delete;
	OrderedLock& operator()(OrderedLock&&)noexcept = delete;
	~OrderedLock();
	void Lock();
	void UnLock();
private:
	struct bucket{
		bucket();
		bucket(const bucket&) = delete;
		bucket(bucket&&) = delete;
		bucket& operator =(const bucket&) = delete;
		bucket& operator =(bucket&&)noexcept = delete;
		bucket& operator ()(const	bucket&) = delete;
		bucket& operator ()(bucket&&)noexcept = delete;
		OrderedLock* self{};
		std::unique_ptr<std::remove_pointer_t< HANDLE>, decltype(CloseHandle)*> hEvent;
	}	*__pBucket;
	MemoryLoan<bucket> __mlBuckets;
	PAPCFUNC const __pAPCLock;
	PAPCFUNC const __pAPCUnLock;
	_beginthreadex_proc_type const __pThreadOperationProc;
	_beginthreadex_proc_type const __pThreadWorkerProc;
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> __hEvWorkerGate;
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> __hEvEndWorkerThread;
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> __hEvEndOpThread;

	HANDLE  __hThreadHost;
	HANDLE __hThreadOp;
	bucket* __pCurrentBucket{};
};

