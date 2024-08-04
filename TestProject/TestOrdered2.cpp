/**
 * @file TestOrderedCout2.cpp
 * @brief ロックの順番保障クラスのデモ
 * @author Gold Smith<br>
 * @date 2022-2024<br>
 * @copyright SPDX-License-Identifier: MIT<br>
 * Released under the MIT license<br>
 * https: //opensource.org/licenses/mit-license.php<br>
 * このファイル内のすべてのコードは、特に明記されていない限り、MITライセンスに従います。
 */
#include <Windows.h>
#include <memory>
#include <iostream>
#include <sstream>
#include <atomic>
#include <thread>
#include <iomanip>
#include <crtdbg.h>
#include "../CommonLib/defSTRINGIZE.h"
#include "../CommonLib/OrderedCout.h"
#include "../CommonLib/OrderedLock.h"
#include "../CommonLib//ordered_lock_cv.h"
#pragma comment(lib,  "../CommonLib/" STRINGIZE($CONFIGURATION) "/CommonLib-" STRINGIZE($CONFIGURATION) ".lib")
#pragma once

using namespace std;

// using lock_class = OrderedLock;// 1
using lock_class = ordered_lock_cv; // 2

static constexpr DWORD SETUP_TIME = 2000;
static constexpr DWORD NUM_THREADS = 5000;
static constexpr DWORD NUM_TIMES = 3;
static constexpr DWORD NUM_PARAMS_UNITS = 0x2000;

HANDLE hEvStart_; // 3
std::condition_variable cv;
std::mutex mtx;

atomic<unsigned> total(1);// 4

struct params{
	lock_class* pOrderLock;
	OrderedCOut* pOrderCout;
	size_t num;
	MemoryLoan<params>* pML;
	int times{};
};// 5

void doWorkInOrder(params* ppms){
	ppms->pOrderLock->Lock(); // 6

	// ここで作業を行う
	ppms->pOrderCout->Push("thread" + to_string(ppms->num) + " "
		+ to_string(ppms->times) + " " + "times" + "       "
		+ "total pass count " + to_string(total) + "\n"); // 7

	++total;
	ppms->pOrderLock->UnLock(); // 6
}

unsigned __stdcall ThreadFunc(void* param){ // 8
	unique_ptr<params, void (*)(params*)> ppms = { reinterpret_cast< params* >(param)
		,[](params* p){p->pML->Return(p); } };


	WaitForSingleObject(hEvStart_, INFINITE); // 9
	for ( int i = 1; i <= NUM_TIMES; ++i ){
		ppms->times = i;
		doWorkInOrder(ppms.get());
	}
	return 0;
}

int main(){ 
	{
		OrderedCOut OrCout; // 10

		unique_ptr<remove_pointer_t<HANDLE>, decltype(CloseHandle)*> hEvStart{ [ & ](){
		if ( !(hEvStart_ = CreateEvent(NULL, TRUE, FALSE, NULL)) ){
			throw std::exception("CreateEvent");
		} return hEvStart_; }(), CloseHandle }; // 11

		OrCout.Trigger(true); // 12

		lock_class lock; // 13

		unique_ptr<params[]> paramsArr = make_unique<params[]>(NUM_PARAMS_UNITS);
		MemoryLoan mlParams(paramsArr.get(), NUM_PARAMS_UNITS);

		unique_ptr<HANDLE[]> threads = make_unique<HANDLE[]>(NUM_THREADS);
		for ( size_t i = 0; i < NUM_THREADS; ++i ){
			params* pPrms = &((*mlParams.Lend()) = { &lock,&OrCout,i,&mlParams,0 });
			if ( !(threads[ i ] = ( HANDLE ) _beginthreadex(
				nullptr, 0, &ThreadFunc, pPrms, 0, nullptr)) ){
				ENOut(errno);
			}
		}

		Sleep(SETUP_TIME); // 14
		SetEvent(hEvStart_); // 15

		for ( size_t i = 0; i < NUM_THREADS; ++i ){ // 16
			DWORD dw;
			if ( !((dw = ::WaitForSingleObject(threads[ i ], INFINITE)) == WAIT_OBJECT_0) ){
				switch ( dw ){
				case WAIT_TIMEOUT:
				{
					_MES("time out" + to_string(i));
					break;
				}
				default:
					ENOut(::GetLastError());
					break;
				}
			}
		}

		for ( size_t i = 0; i < NUM_THREADS; ++i ){ // 17
			CloseHandle(threads[ i ]);
		}

		OrCout.StopTimer();// 18
		stringstream ss;
		ss << "Main:total elapsed time: "
			<< fixed << setprecision(3)
			<< OrCout.TotalTime()
			<< " msec" << endl;
		OrCout.Push(ss.str()); // 19
		OrCout.MessageFlush(); // 20

	}
	_CrtDumpMemoryLeaks();
	return 0;
}
