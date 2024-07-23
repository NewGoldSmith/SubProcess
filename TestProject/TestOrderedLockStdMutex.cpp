/**
 * @file TestOrderedLockStdMutex.cpp
 * @brief ロックの順番保障ミューテックスのデモ
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
#include <mutex>
#include <iomanip>
#include <crtdbg.h>
#include "../CommonLib/defSTRINGIZE.h"
#include "../CommonLib/OrderedCout.h"
#include "../CommonLib/OrderLock.h"
#pragma comment(lib,  "../CommonLib/" STRINGIZE($CONFIGURATION) "/CommonLib-" STRINGIZE($CONFIGURATION) ".lib")
#pragma once

using namespace std;

HANDLE hEvStart_; // 1
OrderedCOut OrCout; // 2
mutex mtx1, mtx2, mtx3; // 3


void doWorkInOrder(int num, int i){ // 4
	mtx1.lock(); // 5
	mtx2.lock(); // 5
	mtx3.lock(); // 5

	// ここで作業を行う
	OrCout.Push("thread" + to_string(num) + " " + to_string(i) + "\n"); // 8

	mtx1.unlock(); // 6
	Sleep(1); // 7
	mtx2.unlock(); // 6
	Sleep(1); // 7
	mtx3.unlock(); // 6
}

void threadFunc(int num){ // 7
	WaitForSingleObject(hEvStart_, INFINITE);// 8
	for( int i = 0; i < 5; ++i ){
		doWorkInOrder(num, i);
	}
}

int main(){ // 9
	{

		unique_ptr<remove_pointer_t<HANDLE>, decltype(CloseHandle)*> hEvStart{ [&](){
		if( !(hEvStart_ = CreateEvent(NULL, TRUE, FALSE, NULL)) ){
			throw std::exception("CreateEvent");
		} return hEvStart_; }(), CloseHandle }; // 10

		OrCout.Trigger(true);

		OrderLock lock;

		thread t1(threadFunc, 1); // 11
		thread t2(threadFunc, 2); // 11
		thread t3(threadFunc, 3); // 11

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
	}
	_CrtDumpMemoryLeaks();
	return 0;
}
