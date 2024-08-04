/**
 * @file ordered_lock_cv.h
 * @brief ordered_lock_cvクラス宣言
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */

#include <memory>
#include <mutex>
#include <atomic>
#include <exception>
#include <condition_variable>
#include <thread>
#include <queue>

#pragma once
class ordered_lock_cv{
	static constexpr unsigned NUM_LOCKS = 0x8000;
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
	std::mutex mtx;
	std::mutex mtx_guard;
	std::condition_variable cv;
	std::queue<std::thread::id> que_ids;
	std::thread::id target_id;
	bool is_locked{ false };
	void (* const p_push_and_acquire)(ordered_lock_cv* pthis, std::thread::id id, bool* pb);
	void (* const p_pop_and__notification)(ordered_lock_cv* pthis);
};

