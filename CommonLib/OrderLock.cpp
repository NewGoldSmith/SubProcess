/**
 * @file OrderLock.cpp
 * @brief OrderLock作成クラス宣言
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#include "OrderLock.h"

OrderLock::OrderLock():

	__pBucket{ new bucket[NUM_LOCK] }

	,__mlBuckets(__pBucket, NUM_LOCK)

	,__hEventEndThread{[](){
		HANDLE h;
		if( !(h = CreateEvent(NULL, TRUE, FALSE, NULL)) ){
			throw std::exception("CreateEvent");
		} return h; }(), CloseHandle }

	, __hEventHost{ [](){
		HANDLE h;
		if( !(h = CreateEvent(NULL, TRUE, FALSE, NULL)) ){
			throw std::exception("CreateEvent");
		} return h; }(), CloseHandle }
	
	, __pAPCCallBack{ [](ULONG_PTR Parameter){
		bucket *pBucket = reinterpret_cast<bucket*>(Parameter);
		SetEvent(pBucket->hEvent.get());
		ResetEvent(pBucket->self->__hEventHost.get());
		WaitForSingleObject(pBucket->self->__hEventHost.get(), INFINITE);
	} }

	, __pThreadWarkerProc{ [](LPVOID pvoid)->DWORD{

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
	if( !(__hThreadHost = CreateThread(
		NULL
		, 0
		, __pThreadWarkerProc
		, __hEventEndThread.get()
		, 0
		, NULL)) ){
		throw std::exception("CreateThread");
	};
}

OrderLock::~OrderLock(){
	SetEvent(__hEventEndThread.get());
	WaitForSingleObject(__hThreadHost, INFINITE);
	delete[]__pBucket;
}

void OrderLock::Lock(){
	bucket* pBucket = __mlBuckets.Lend();
	pBucket->self = this;
	pBucket->hThreadGest = GetCurrentThread();
	ResetEvent(pBucket->hEvent.get());
	QueueUserAPC(__pAPCCallBack, __hThreadHost, (ULONG_PTR)pBucket);
	WaitForSingleObject(pBucket->hEvent.get(),INFINITE);
	return ;
}

void OrderLock::UnLock(){
	SetEvent(__hEventHost.get());
	__mlBuckets.Return(__pCurrentBucket);
}

OrderLock::bucket::bucket():
	hEvent{ [](){HANDLE h; if( !(h = CreateEvent(NULL,TRUE,FALSE,NULL)) ){
		std::string str = debug_fnc::ENOut(GetLastError());
		throw std::exception(str.c_str());}	return h;}()
	,CloseHandle }

{}

