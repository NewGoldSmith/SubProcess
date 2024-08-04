/**
 * @file OrderedLock.cpp
 * @brief OrderLock作成クラス宣言
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#include "OrderedLock.h"

OrderedLock::OrderedLock():

	__pBucket{ new bucket[NUM_LOCKS]}

	,__mlBuckets(__pBucket, NUM_LOCKS)

	,__hEvEndWorkerThread{[](){
		HANDLE h;
		if( !(h = CreateEvent(NULL, TRUE, FALSE, NULL)) ){
			throw std::exception(_MES("CreateEvent").c_str());
		} return h; }(), CloseHandle }

	, __hEvEndOpThread{ [](){
		HANDLE h;
		if( !(h = CreateEvent(NULL, TRUE, FALSE, NULL)) ){
			throw std::exception(_MES("CreateEvent").c_str());
		} return h; }(), CloseHandle }

	, __hEvWorkerGate{ [](){
		HANDLE h;
		if( !(h = CreateEvent(NULL, TRUE, FALSE, NULL)) ){
			throw std::exception(_MES("CreateEvent").c_str());
		} return h; }(), CloseHandle }

	, __pAPCLock{ [](ULONG_PTR Parameter){
		bucket *pBucket = reinterpret_cast<bucket*>(Parameter);
		pBucket->self->__pCurrentBucket = pBucket;
		ResetEvent(pBucket->self->__hEvWorkerGate.get());
		SetEvent(pBucket->hEvent.get());
		WaitForSingleObject(pBucket->self->__hEvWorkerGate.get(), INFINITE);
	} }

	, __pAPCUnLock{ [](ULONG_PTR Parameter){
		bucket* pBucket = reinterpret_cast<bucket*>(Parameter);
		pBucket->self->__mlBuckets.Return(pBucket);
		SetEvent(pBucket->self->__hEvWorkerGate.get());
	} }

	, __pThreadOperationProc{ [](LPVOID pvoid)->unsigned{

		OrderedLock* pThis = reinterpret_cast<OrderedLock*>(pvoid);
		for( ;;){
			DWORD dw = ::WaitForSingleObjectEx(pThis->__hEvEndOpThread.get(), INFINITE, TRUE);
			switch( dw ){
				case WAIT_IO_COMPLETION:
				{
					continue;
				}
				case WAIT_OBJECT_0:
				{
					_D("OperationProc end.");
					return 0;
				}
				default:
					throw std::exception(_MES("__pThreadOperationProc").c_str());
			}
		}
	} }

	, __pThreadWorkerProc{ [](LPVOID pvoid)->unsigned{

		OrderedLock* pThis = reinterpret_cast<OrderedLock*>(pvoid);

		for( ;;){
			DWORD dw = ::WaitForSingleObjectEx(pThis->__hEvEndWorkerThread.get(), INFINITE, TRUE);
			switch( dw ){
				case WAIT_IO_COMPLETION:
				{
					continue;
				}
				case WAIT_OBJECT_0:
				{
					_D("WorkerProc end.");
					return 0;
				}
				default:
					throw std::exception(_MES("__pThreadWorkerProc").c_str());
			}
		}
	} }
{
	if( !(__hThreadOp = (HANDLE)_beginthreadex(
		NULL
		, 0
		, __pThreadOperationProc
		, this
		, 0
		, NULL)) ){
		throw std::exception(_MES("CreateThread").c_str());
	};

	if( !(__hThreadHost = (HANDLE)_beginthreadex(
		NULL
		, 0
		, __pThreadWorkerProc
		, this
		, 0
		, NULL)) ){
		throw std::exception(_MES("CreateThread").c_str());
	};
}

OrderedLock::~OrderedLock(){
	::SetEvent(__hEvEndWorkerThread.get());
	::SetEvent(__hEvEndOpThread.get());
	::WaitForSingleObject(__hThreadHost, INFINITE);
	::CloseHandle(__hThreadHost);
	::WaitForSingleObject(__hThreadOp, INFINITE);
	::CloseHandle(__hThreadOp);
	delete[]__pBucket;
}

void OrderedLock::Lock(){
	bucket* pBucket = __mlBuckets.Lend();
	pBucket->self = this;
	::ResetEvent(pBucket->hEvent.get());
	::QueueUserAPC(__pAPCLock, __hThreadHost, (ULONG_PTR)pBucket);
	::WaitForSingleObject(pBucket->hEvent.get(),INFINITE);
	return ;
}

void OrderedLock::UnLock(){
	::QueueUserAPC(__pAPCUnLock, __hThreadOp, (ULONG_PTR)__pCurrentBucket);
}

OrderedLock::bucket::bucket():
	hEvent{ [](){HANDLE h; if( !(h = CreateEvent(NULL,TRUE,FALSE,NULL)) ){
		throw std::exception(ENOut(GetLastError()).c_str());}	return h;}()
	,CloseHandle }

{}

