/**
 * @file OrderLock.cpp
 * @brief OrderLock作成クラス宣言
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#include "OrderLock.h"

OrderLock::OrderLock():

	__pBucket{ new bucket[NUM_LOCK]}

	,__mlBuckets(__pBucket, NUM_LOCK)

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

	, __hEvFlush{ [](){
		HANDLE h;
		if( !(h = CreateEvent(NULL, TRUE, FALSE, NULL)) ){
			throw std::exception(_MES("CreateEvent").c_str());
		} return h; }(), CloseHandle }

	, __hEvWaitForWorker{ [](){
		HANDLE h;
		if( !(h = CreateEvent(NULL, TRUE, FALSE, NULL)) ){
			throw std::exception(_MES("CreateEvent").c_str());
		} return h; }(), CloseHandle }

	, __hEvGuard{ [](){
		HANDLE h;
		if( !(h = CreateEvent(NULL, TRUE, FALSE, NULL)) ){
			throw std::exception(_MES("CreateEvent").c_str());
		} return h; }(), CloseHandle }


	, __pOpFlush{ [](ULONG_PTR Parameter){
		OrderLock* pthis = reinterpret_cast<OrderLock*>(Parameter);
	} }

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

		OrderLock* pThis = reinterpret_cast<OrderLock*>(pvoid);
		for( ;;){
			DWORD dw = ::WaitForSingleObjectEx(pThis->__hEvEndOpThread.get(), INFINITE, TRUE);
			switch( dw ){
				case WAIT_IO_COMPLETION:
				{
					_D("OperationProc executed.");
					continue;
				}
				case WAIT_OBJECT_0:
				{
					_D("end.");
					return 0;
				}
				default:
					throw std::exception(_MES("__pThreadOperationProc").c_str());
			}
		}
	} }

	, __pThreadWorkerProc{ [](LPVOID pvoid)->unsigned{

		OrderLock* pThis = reinterpret_cast<OrderLock*>(pvoid);
		unsigned cnt = 0;


		for( ;;){
			DWORD dw = ::WaitForSingleObjectEx(pThis->__hEvEndWorkerThread.get(), INFINITE, TRUE);
			switch( dw ){
				case WAIT_IO_COMPLETION:
				{
					_D("WorkerProc executed."+ std::to_string(cnt));
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
	//hEvents.hEvEnd = __hEvEndWorkerThread.get();
	//hEvents.hEvFlush = __hEvFlush.get();
	//hEvents.hEvWait = __hEvWaitForWorker.get();
	//hEvents.self = this;

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

OrderLock::~OrderLock(){
	::SetEvent(__hEvEndWorkerThread.get());
	SetEvent(__hEvEndOpThread.get());
	//WaitForSingleObject(__hEvWaitForWorker.get(), INFINITE);
	WaitForSingleObject(__hThreadHost, INFINITE);
	::CloseHandle(__hThreadHost);
	WaitForSingleObject(__hThreadOp, INFINITE);
	CloseHandle(__hThreadOp);
	delete[]__pBucket;
}

void OrderLock::Lock(){
	bucket* pBucket = __mlBuckets.Lend();
	pBucket->self = this;
	pBucket->hThreadGest = GetCurrentThread();
	ResetEvent(pBucket->hEvent.get());
	QueueUserAPC(__pAPCLock, __hThreadHost, (ULONG_PTR)pBucket);
	WaitForSingleObject(pBucket->hEvent.get(),INFINITE);
	return ;
}

void OrderLock::UnLock(){
	QueueUserAPC(__pAPCUnLock, __hThreadOp, (ULONG_PTR)__pCurrentBucket);
}

void OrderLock::Flush(){
}

void OrderLock::__Flush(){
	std::unique_ptr < std::remove_pointer_t<HANDLE>, decltype(::CloseHandle)* > hEvCleanUp
	{ [](){HANDLE h; if( !(h = CreateEvent(NULL, TRUE, FALSE, NULL)) ){
			throw std::exception(_MES("APC hEvCleanUp").c_str());}return h; }()
				,	CloseHandle };

	for( ;; ){
		DWORD dw2 = ::WaitForSingleObjectEx(hEvCleanUp.get(), TIME_OUT, TRUE);
		switch( dw2 ){
			case WAIT_IO_COMPLETION:
			{
				_D("Cleanup phase executed.");
				::SleepEx(0, TRUE);
				continue;
			}
			case WAIT_TIMEOUT:
			{
				_D("The queue has been emptied.");
				return ;
			}
			case WAIT_OBJECT_0:
			{
				throw std::exception(_MES("__Flush err.").c_str());
			}
			default:
			{
				throw std::exception(_MES("__Flush err.").c_str());
			}
		}
	}
}

OrderLock::bucket::bucket():
	hEvent{ [](){HANDLE h; if( !(h = CreateEvent(NULL,TRUE,FALSE,NULL)) ){
		throw std::exception(ENOut(GetLastError()).c_str());}	return h;}()
	,CloseHandle }

{}

