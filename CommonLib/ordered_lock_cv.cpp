/**
 * @file ordered_lock_cv.cpp
 * @brief ordered_lock_cvクラス実装
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#include "ordered_lock_cv.h"

ordered_lock_cv::ordered_lock_cv():

	 p_push_and_acquire{ [](
		ordered_lock_cv* pthis
		, std::thread::id id
		,bool* pb){
		
		std::lock_guard < std::mutex> lk(pthis->mtx_guard);
		if( !pthis->is_locked ){
			pthis->is_locked = true;
			*pb = true;
		} else{
			*pb = false;
			pthis->que_ids.push(id);
		}
	
	} }

	, p_pop_and__notification{ [](ordered_lock_cv* pthis){
		std::lock_guard < std::mutex> lk(pthis->mtx_guard);
		if( pthis->que_ids.size() ){
			pthis->target_id = pthis->que_ids.front();
			pthis->que_ids.pop();
			pthis->cv.notify_all();
			return;
		} else{
			pthis->is_locked = false;
		}
	} }
{
}

ordered_lock_cv::~ordered_lock_cv(){
}

void ordered_lock_cv::Lock(){
	std::thread::id id = std::this_thread::get_id();
	bool is_acquire{ false };
	{
		std::thread thread(p_push_and_acquire, this, id ,&is_acquire);
		thread.join();
	}
	if( is_acquire ){
		return;
	}else	{
		std::unique_lock<std::mutex> mtx_lk(mtx);
		cv.wait(mtx_lk, [this, id]{return id == target_id; });
	}
	return;
}

void ordered_lock_cv::UnLock(){
	std::thread thread(p_pop_and__notification, this);
	thread.join();
}
