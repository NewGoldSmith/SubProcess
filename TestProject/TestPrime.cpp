/**
 * @file TestPrime.cpp
 * @brief APCを使ったサンプルコード実装
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <memory>
#include <cmath>
#include <crtdbg.h>
#include <exception>
using namespace std;
using  psh_t = pair<stringstream*, HANDLE>;// 1

bool isPrime(int num){ // 2
	if( num <= 1 )
		return false;
	if( num == 2 )
		return true;
	if( num % 2 == 0 )
		return false;
	for( int i = 3, sqnum = (int)sqrt(num); i <= sqnum; i += 2 ){
		if( num % i == 0 )
			return false;
	}
	return true;
}

void GetPrimeFrom1To10000(ULONG_PTR p){ // 3
	psh_t* psh = reinterpret_cast<psh_t*>(p);
	for( int num(1); num <= 10000; ++num ){
		if( isPrime(num) ){
			*(psh->first) << num << " ";
		}
	}
	if( psh->second )
		SetEvent(psh->second); // 4
}

void GetPrimeFrom10001To20000(ULONG_PTR p){ // 5
	psh_t* psh = reinterpret_cast<psh_t*>(p);
	for( int num(10001); num <= 20000; ++num ){
		if( isPrime(num) ){
			*(psh->first) << num << " ";
		}
	}
	if( psh->second )
		SetEvent(psh->second); // 4
}

void GetPrimeFrom20001To30000(ULONG_PTR p){ // 6
	psh_t* psh = reinterpret_cast<psh_t*>(p);
	for( int num(20001); num <= 30000; ++num ){
		if( isPrime(num) ){
			*(psh->first) << num << " ";
		}
	}
	if( psh->second )
		SetEvent(psh->second); // 4
}

void GetPrimeFrom30001To40000(ULONG_PTR p){ // 7
	psh_t* psh = reinterpret_cast<psh_t*>(p);
	for( int num(30001); num <= 40000; ++num ){
		if( isPrime(num) ){
			*(psh->first) << num << " ";
		}
	}
	if( psh->second )
		SetEvent(psh->second); // 4
}

DWORD pThreadProc(LPVOID pvoid){ // 8
	HANDLE hEvent = reinterpret_cast<HANDLE>(pvoid);
	for( ;;){
		DWORD dw = ::WaitForSingleObjectEx(hEvent, INFINITE, TRUE);
		if( dw == WAIT_IO_COMPLETION ){
			OutputDebugStringA("APC executed.\r\n");
		} else if( dw == WAIT_OBJECT_0 ){
			OutputDebugStringA("Event signaled.\r\n");
			return 0;
		} else{
			cerr << "err" << endl;
			return 123;
		}
	}
}

int main(){
	{
		unique_ptr<remove_pointer_t<HANDLE>, decltype(CloseHandle)*> hEvThread{ [](){
			HANDLE h;
		if( !(h = CreateEvent(NULL, TRUE, FALSE, NULL)) ){
			throw exception("CreateEvent");
		} return h; }(),CloseHandle }; // 9

		unique_ptr<remove_pointer_t<HANDLE>, decltype(CloseHandle)*> hEvPrime{ [](){
			HANDLE h;
		if( !(h = CreateEvent(NULL, TRUE, FALSE, NULL)) ){
			throw exception("CreateEvent");
		} return h; }(),CloseHandle }; // 10

		unique_ptr<remove_pointer_t<HANDLE>, decltype(CloseHandle)*> hThread{ [&](){
		  HANDLE h;
		  if( !(h = CreateThread(NULL,0,pThreadProc,hEvThread.get(), 0, NULL)) ){
			  throw exception("CreateThread");
		  } return h; }(),CloseHandle }; //11

		stringstream ss1, ss2;
		psh_t pairss{ &ss1,NULL };
		psh_t pairssev{ &ss2,hEvPrime.get() };
		QueueUserAPC((PAPCFUNC)GetPrimeFrom10001To20000, hThread.get(), (ULONG_PTR)&pairssev); // 12
		GetPrimeFrom1To10000((ULONG_PTR)&pairss); // 13
		WaitForSingleObject(hEvPrime.get(), INFINITE); // 14

		stringstream ss3, ss4;
		pairss.first = &ss3;
		ResetEvent(hEvPrime.get()); // 15
		pairssev.first = &ss4;
		QueueUserAPC((PAPCFUNC)GetPrimeFrom30001To40000, hThread.get(), (ULONG_PTR)&pairssev); // 16
		GetPrimeFrom20001To30000((ULONG_PTR)&pairss); // 17
		WaitForSingleObject(hEvPrime.get(), INFINITE); // 18

		cout << ss1.str() << endl;
		cout << ss2.str() << endl;
		cout << ss3.str() << endl;
		cout << ss4.str() << endl;

		SetEvent(hEvThread.get()); // 19
		WaitForSingleObject(hThread.get(), INFINITE); // 20
	}
	_CrtDumpMemoryLeaks();
	return 0;
}
