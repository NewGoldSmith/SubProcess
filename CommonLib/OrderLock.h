/**
 * @file OrderLock.h
 * @brief OrderLock作成クラス宣言
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */

#include <Windows.h>
#include <process.h>
#include <iostream>
#include <memory>
#include <type_traits>
#include <mutex>
#include <exception>
#include "../CommonLib/MemoryLoan.h"
#include "../Debug_fnc/debug_fnc.h"

#pragma once
class OrderLock{
	static constexpr DWORD NUM_LOCK = 0x8000;
	static constexpr DWORD TIME_OUT = 100;
public:
	OrderLock();
	OrderLock(const OrderLock&) = delete;
	OrderLock& operator=(const OrderLock&) = delete;
	OrderLock& operator()(const OrderLock&) = delete;
	OrderLock(OrderLock&&)noexcept = delete;
	OrderLock& operator=(OrderLock&&)noexcept = delete;
	OrderLock& operator()(OrderLock&&)noexcept = delete;
	~OrderLock();
	void Lock();
	void UnLock();
	void Flush();
private:
	struct bucket{
		bucket();
		bucket(const bucket&) = delete;
		bucket(bucket&&) = delete;
		bucket& operator =(const bucket&) = delete;
		bucket& operator =(bucket&&)noexcept = delete;
		bucket& operator ()(const	bucket&) = delete;
		bucket& operator ()(bucket&&)noexcept = delete;
		OrderLock* self{};
		HANDLE hThreadGest{};
		std::unique_ptr<std::remove_pointer_t< HANDLE>, decltype(CloseHandle)*> hEvent;
	}	*__pBucket;
	MemoryLoan<bucket> __mlBuckets;
	//struct hEvents_t{
	//	HANDLE hEvEnd{};
	//	HANDLE hEvFlush{};
	//	HANDLE hEvWait{};
	//	OrderLock* self{};
	//} hEvents;
	PAPCFUNC const __pAPCLock;
	PAPCFUNC const __pAPCUnLock;
	PAPCFUNC const __pOpFlush;
	_beginthreadex_proc_type const __pThreadOperationProc;
	_beginthreadex_proc_type const __pThreadWorkerProc;
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> __hEvWorkerGate;
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> __hEvEndWorkerThread;
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> __hEvEndOpThread;
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> __hEvFlush;
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> __hEvWaitForWorker;
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> __hEvGuard;

	HANDLE  __hThreadHost;
	HANDLE __hThreadOp;
	bucket* __pCurrentBucket{};
	void __Flush();
};

