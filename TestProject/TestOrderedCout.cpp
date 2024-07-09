/**
 * @file TestOrderedCout.cpp
 * @brief OrderedCoutのtestコード実装
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#include "TestOrderedCout.h"
#include <thread>
#include <memory>
using namespace std;
void OrderedFnc(OrderedCOut* pOC, HANDLE h, const int i) {// 1
	string str = "test" + to_string(i);// 2
	::WaitForSingleObject(h, INFINITE);// 3
	pOC->Push(str);// 4
}

int main() {
	{
		unique_ptr<remove_pointer_t<HANDLE>, decltype(::CloseHandle)*> hEvent = {
			[]() {return ::CreateEvent(NULL,TRUE,FALSE,NULL); }(),::CloseHandle };// 5

		OrderedCOut oc;
		
		thread t0(OrderedFnc, &oc, hEvent.get(), 0);// 6
		thread t1(OrderedFnc, &oc, hEvent.get(), 1);// 6
		thread t2(OrderedFnc, &oc, hEvent.get(), 2);// 6
		thread t3(OrderedFnc, &oc, hEvent.get(), 3);// 6
		thread t4(OrderedFnc, &oc, hEvent.get(), 4);// 6
		thread t5(OrderedFnc, &oc, hEvent.get(), 5);// 6
		thread t6(OrderedFnc, &oc, hEvent.get(), 6);// 6
		thread t7(OrderedFnc, &oc, hEvent.get(), 7);// 6
		thread t8(OrderedFnc, &oc, hEvent.get(), 8);// 6
		thread t9(OrderedFnc, &oc, hEvent.get(), 9);// 6
		thread t10(OrderedFnc, &oc, hEvent.get(), 10);// 6
		thread t11(OrderedFnc, &oc, hEvent.get(), 11);// 6

		oc.Trigger(true);// 7
		Sleep(1000);// 8
		::SetEvent(hEvent.get());// 9
		oc.MessageFlush();
		oc.StopTimer();
		oc.ShowTimeDisplay(false);
		oc.MessageFlush();
		oc.Push("TotalTime:" + to_string(oc.TotalTime()) + " msec\n");


		t0.join();// 10
		t1.join();// 10
		t2.join();// 10
		t3.join();// 10
		t4.join();// 10
		t5.join();// 10
		t6.join();// 10
		t7.join();// 10
		t8.join();// 10
		t9.join();// 10
		t10.join();// 10
		t11.join();// 10
	}
	_CrtDumpMemoryLeaks();
	return 0;
}
