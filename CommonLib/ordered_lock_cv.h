/**
 * @file ordered_lock_cv.h
 * @brief ordered_lock_cvクラス宣言
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#include <Windows.h>
#include <iostream>
#include <memory>
#include <type_traits>
#include <exception>
#include <condition_variable>
#include <mutex>
#include "../CommonLib/MemoryLoan.h"
#include "../Debug_fnc/debug_fnc.h"

#pragma once
class ordered_lock_cv{
	static constexpr DWORD NUM_LOCK = 0x8000;
public:
	ordered_lock_cv();
	ordered_lock_cv(const ordered_lock_cv&) = delete;
	ordered_lock_cv& operator=(const ordered_lock_cv&) = delete;
	ordered_lock_cv& operator()(const ordered_lock_cv&) = delete;
	ordered_lock_cv(ordered_lock_cv&&)noexcept = delete;
	ordered_lock_cv& operator=(ordered_lock_cv&&)noexcept = delete;
	ordered_lock_cv& operator()(ordered_lock_cv&&)noexcept = delete;
	~ordered_lock_cv();
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
		ordered_lock_cv* self{};
		HANDLE hThreadGest{};
		std::unique_ptr<std::remove_pointer_t< HANDLE>, decltype(CloseHandle)*> hEvent;
	};
	bucket* __pBucket{};
	MemoryLoan<bucket> __mlBuckets;
	PAPCFUNC const __pAPCCallBack;
	PAPCFUNC const __pAPCLockUnLock;
	LPTHREAD_START_ROUTINE const __pThreadWorkerProc;
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> __hEventHost;
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> __hEventEndThread;
	HANDLE __hThreadHost;
	bucket* __pCurrentBucket{};
	std::mutex mtx;
	std::condition_variable cv;
};
