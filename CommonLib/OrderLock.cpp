/**
 * @file OrderLock.h
 * @brief OrderLock作成クラス宣言
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#include "OrderLock.h"

OrderLock::OrderLock():

	pBucket{ new bucket[NUM_LOCK] }

	,mlBuckets(pBucket, NUM_LOCK)

	,hEventEndThread{[](){
		HANDLE h;
		if( !(h = CreateEvent(NULL, TRUE, FALSE, NULL)) ){
			throw std::exception("CreateEvent");
		} return h; }(), CloseHandle }

	, hEventHost{ [](){
		HANDLE h;
		if( !(h = CreateEvent(NULL, TRUE, FALSE, NULL)) ){
			throw std::exception("CreateEvent");
		} return h; }(), CloseHandle }
	
	, __pAPCProc{ [](ULONG_PTR Parameter){
		bucket *pBucket = reinterpret_cast<bucket*>(Parameter);
		SetEvent(pBucket->hEvent.get());
		ResetEvent(pBucket->self->hEventHost.get());
		WaitForSingleObject(pBucket->self->hEventHost.get(), INFINITE);
	} }

	, pThreadProc{ [](LPVOID pvoid)->DWORD{

		HANDLE hEvent = reinterpret_cast<HANDLE>(pvoid);
		for( ;;){
			DWORD dw = ::WaitForSingleObjectEx(hEvent, INFINITE, TRUE);
			if( dw == WAIT_IO_COMPLETION ){
				OutputDebugStringA("APC executed.\r\n");
			} else if( dw == WAIT_OBJECT_0 ){
				OutputDebugStringA("Event signaled.\r\n");
				return 0;
			} else{
				std::cerr << "err" << std::endl;
				return 123;
			}
		}
	} }
{
	if( !(hThreadHost = CreateThread(
		NULL
		, 0
		, pThreadProc
		, hEventEndThread.get()
		, 0
		, NULL)) ){
		throw std::exception("CreateThread");
	};
}

OrderLock::~OrderLock(){
	SetEvent(hEventEndThread.get());
	WaitForSingleObject(hThreadHost, INFINITE);
	delete[]pBucket;
}

void OrderLock::Lock(){
	bucket* pBucket = mlBuckets.Lend();
	pBucket->self = this;
	pBucket->hThreadGest = GetCurrentThread();
	ResetEvent(pBucket->hEvent.get());
	QueueUserAPC(__pAPCProc, hThreadHost, (ULONG_PTR)pBucket);
	WaitForSingleObject(pBucket->hEvent.get(),INFINITE);
	return ;
}

void OrderLock::UnLock(){
	SetEvent(hEventHost.get());
	mlBuckets.Return(pCurrentBucket);
}

OrderLock::bucket::bucket():
	hEvent{ [](){HANDLE h; if( !(h = CreateEvent(NULL,TRUE,FALSE,NULL)) ){
		std::string str = debug_fnc::ENOut(GetLastError());
		throw std::exception(str.c_str());}	return h;}()
	,CloseHandle }

{}

