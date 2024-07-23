/**
 * @file OrderLock.h
 * @brief OrderLock作成クラス宣言
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */

#include <Windows.h>
#include <iostream>
#include <memory>
#include <type_traits>
#include <exception>
#include "../CommonLib/MemoryLoan.h"
#include "../Debug_fnc/debug_fnc.h"

#pragma once
class OrderLock{
	static constexpr DWORD NUM_LOCK = 0x40;
	struct bucket;
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
		std::unique_ptr<std::remove_pointer_t< HANDLE>,decltype(CloseHandle)*> hEvent;
	};
	bucket* pBucket{};
	MemoryLoan<bucket> mlBuckets;
	PAPCFUNC const __pAPCProc;
	LPTHREAD_START_ROUTINE const pThreadProc;
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> hEventHost;
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> hEventEndThread;
	HANDLE hThreadHost;
	bucket* pCurrentBucket{};

};

