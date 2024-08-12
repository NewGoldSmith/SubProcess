/**
 * @file TestOrdered3.cpp
 * @brief ロックの順番保障クラスのデモ
 * @author Gold Smith<br>
 * @date 2022-2024<br>
 * @copyright SPDX-License-Identifier: MIT<br>
 * Released under the MIT license<br>
 * https: //opensource.org/licenses/mit-license.php<br>
 * このファイル内のすべてのコードは、特に明記されていない限り、MITライセンスに従います。
 * oneAPIはIntel®の使用条件に従います
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
#include <oneapi/tbb/queuing_mutex.h>
#include <conio.h>
#pragma comment(lib,  "../CommonLib/" STRINGIZE($CONFIGURATION) "/CommonLib-" STRINGIZE($CONFIGURATION) ".lib")
#pragma once

using namespace std;

oneapi::tbb::queuing_mutex  mtx; // 1

static constexpr DWORD SETUP_TIME = 500; // 2
static constexpr DWORD NUM_THREADS = 6;
static constexpr DWORD NUM_TIMES = 3;
static constexpr DWORD NUM_PARAMS_UNITS = 0x1000;

HANDLE hEvStart_; // 3

atomic<unsigned> total(1);// 4

struct params{ // 5
	OrderedCOut* pOrderCout;
	size_t num;
	MemoryLoan<params>* pML;
	int times{};
};

void doWorkInOrder(params* ppms){
	oneapi::tbb::queuing_mutex::scoped_lock lk; // 6
	lk.acquire(mtx); // 7

	// ここで作業を行う
	ppms->pOrderCout->Push("thread" + to_string(ppms->num) + " "
		+ to_string(ppms->times) + " " + "times" + "       "
		+ "total pass count " + to_string(total) + "\n"); // 8

	++total;
}

unsigned __stdcall ThreadFunc(void* param){ // 9
	unique_ptr<params, void (*)(params*)> ppms = { reinterpret_cast< params* >(param)
		,[](params* p){p->pML->Return(p); } };


	WaitForSingleObject(hEvStart_, INFINITE); // 10
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

		unique_ptr<params[]> paramsArr = make_unique<params[]>(NUM_PARAMS_UNITS);
		MemoryLoan mlParams(paramsArr.get(), NUM_PARAMS_UNITS); // 13

		unique_ptr<HANDLE[]> threads = make_unique<HANDLE[]>(NUM_THREADS);
		for ( size_t i = 0; i < NUM_THREADS; ++i ){
			params* pPrms = &((*mlParams.Lend()) = {&OrCout,i,&mlParams,0 });
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
				ENOut(::GetLastError());
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
	( void ) _getch();
	return 0;
}
