/**
 * @file TestOrderedCout.cpp
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
#include <thread>
#include <iomanip>
#include <crtdbg.h>
#include "../CommonLib/defSTRINGIZE.h"
#include "../CommonLib/OrderedCout.h"
#include "../CommonLib/OrderLock.h"
#pragma comment(lib,  "../CommonLib/" STRINGIZE($CONFIGURATION) "/CommonLib-" STRINGIZE($CONFIGURATION) ".lib")
#pragma once

using namespace std;

HANDLE hEvStart_; // 1

void doWorkInOrder(OrderLock& lock, OrderedCOut& COut, int num, int i){ // 2
	lock.Lock(); // 3

	// ここで作業を行う
	COut.Push("thread" + to_string(num) + " " + to_string(i) + "\n"); // 4

	lock.UnLock(); // 3
}

void threadFunc(OrderLock& lock, OrderedCOut& OrCOut, int num){ // 5
	WaitForSingleObject(hEvStart_, INFINITE);
	for( int i = 0; i < 5; ++i ){
		doWorkInOrder(ref(lock), ref(OrCOut), num, i);
	}
}

int main(){ // 6
	{
		OrderedCOut OrCout; // 7

		unique_ptr<remove_pointer_t<HANDLE>, decltype(CloseHandle)*> hEvStart{ [&](){
		if( !(hEvStart_ = CreateEvent(NULL, TRUE, FALSE, NULL)) ){
			throw std::exception("CreateEvent");
		} return hEvStart_; }(), CloseHandle }; // 8

		OrCout.Trigger(true); // 9

		OrderLock lock; // 10

		thread t1(threadFunc, ref(lock), ref(OrCout), 1); // 11
		thread t2(threadFunc, ref(lock), ref(OrCout), 2); // 11
		thread t3(threadFunc, ref(lock), ref(OrCout), 3); // 11

		Sleep(200); // 12
		SetEvent(hEvStart_); // 13

		t1.join(); // 14
		t2.join(); // 14
		t3.join(); // 14

		OrCout.StopTimer();// 15
		stringstream ss;
		ss << "Main:total elapsed time: "
			<< fixed << setprecision(3)
			<< OrCout.TotalTime()
			<< " msec" << endl;
		OrCout.Push(ss.str()); // 16
		OrCout.MessageFlush(); // 17
	}
	_CrtDumpMemoryLeaks();
	return 0;
}
