/**
 * @file TestUnOrderedCout.cpp
 * @brief UnOrderedCoutのtestコード実装
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#include <Windows.h>
#include <thread>
#include <memory>
#include <iostream>
#include <string>
using namespace std;

void UnOrderedFnc(HANDLE h, int i) {// 1
	std::string str("test" + to_string(i) + "\n");
	WaitForSingleObject(h, INFINITE);// 2
	cout << str;// 3
}

int main() {
	{
		unique_ptr<remove_pointer_t<HANDLE>, decltype(CloseHandle)*> hEvent = {
			[]() {return CreateEvent(NULL,TRUE,FALSE,NULL); }(),CloseHandle };// 4

		thread t1(UnOrderedFnc, hEvent.get(), 1);// 5
		thread t2(UnOrderedFnc, hEvent.get(), 2);// 5
		thread t3(UnOrderedFnc, hEvent.get(), 3);// 5
		thread t4(UnOrderedFnc, hEvent.get(), 4);// 5
		thread t5(UnOrderedFnc, hEvent.get(), 5);// 5
		thread t6(UnOrderedFnc, hEvent.get(), 6);// 5
		Sleep(100);
		SetEvent(hEvent.get());// 6

		t1.join();// 7
		t2.join();// 7
		t3.join();// 7
		t4.join();// 7
		t5.join();// 7
		t6.join();// 7
	}
	_CrtDumpMemoryLeaks();
	return 0;
}
