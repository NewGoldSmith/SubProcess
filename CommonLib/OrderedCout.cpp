/**
 * @file OrderedCout.cpp
 * @brief OrderedCoutクラス実装
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#include "OrderedCout.h"

OrderedCOut::OrderedCOut() :
	hEventThread{ []() {HANDLE h;
		if (!(h = ::CreateEvent(NULL,TRUE,FALSE,NULL))) {
			std::string str = ENOut(::GetLastError());
			throw std::runtime_error(str);
		};
		return h; }()
	,
		::CloseHandle }
	
	, hEventMessage{ []() {HANDLE h;
		if (!(h = ::CreateEvent(NULL,TRUE,FALSE,NULL))) {
			std::string str = ENOut(::GetLastError());
			throw std::runtime_error(str);
		};
		return h; }()
	,
		::CloseHandle }

	, __MessageArr{ std::make_unique<message[]>(UNIT_SIZE) }

	, __mlms(__MessageArr.get(), UNIT_SIZE)

	, __pAPCProc{ [](ULONG_PTR dwParam) {
		std::unique_ptr<message ,void(*)(message*)> pmes = {
			reinterpret_cast<message*>(dwParam)
			, [](message* p)->void {p->self->__mlms.Return(p); } };

		switch (pmes->op) {
		case OP::NOOP:
			break;
		case OP::SIG_EVENT:
		{
			SetEvent(pmes->hEvent);
			return;
		}
		case OP::START_DISPLAY_TIME:
		{
			pmes->self->bIsDisplayTime = true;
			return;
		}
		case OP::STOP_DISPLAY_TIME:
		{
			pmes->self->bIsDisplayTime = false;
			return;
		}
		case OP::START_TIMER:
		{
			LARGE_INTEGER li;
			::QueryPerformanceCounter(&li);
			pmes->self->__StartingTime = li.QuadPart;
			pmes->self->__GapTime = pmes->self->__StartingTime;
			pmes->self->bIsMeasuring = true;
			return;
		}
		case OP::STOP_TIMER:
		{
			pmes->self->bIsMeasuring = false;
			return;
		}
		case OP::RESUME_TIMER:
		{
			pmes->self->bIsMeasuring = true;
			return;
		}
		case OP::RESET:
		{
			pmes->self->__StartingTime = 0;
			pmes->self->__GapTime = 0;
			pmes->self->bTrigger = true;
			pmes->self->bIsMeasuring = true;
			pmes->self->bIsDisplayTime = true;
			return;
		}
		case OP::START_TRIGGER:
		{
			pmes->self->bTrigger = true;
			return;
		}
		case OP::STOP_TRIGGER:
		{
			pmes->self->bTrigger = false;
			return;
		}
		case OP::TOTAL_TIME:
		{
			LARGE_INTEGER Frequency{};
			::QueryPerformanceFrequency(&Frequency);
			LARGE_INTEGER ElapsedMicroseconds{};
			ElapsedMicroseconds.QuadPart = pmes->self->__GapTime - pmes->self->__StartingTime;
			ElapsedMicroseconds.QuadPart *= 1000000;
			ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
			*reinterpret_cast<double*>(pmes->pvoid) = ElapsedMicroseconds.QuadPart / 1000.0;
			::SetEvent(pmes->hEvent);
			return;
		}
		default:
			break;
		}

		std::string str(pmes->buffer, pmes->size);
		// 最後の文字が'\n'であるか確認
		if (!str.empty() && str[str.size() - 1] == '\n') {
			// '\n'を取り除く
			str.erase(str.size() - 1);
		}

		// メッセージを40文字になるようにパディング
		std::string paddedMessage = str;
		if (paddedMessage.size() < MESSAGE_SPACE_WIDTH) {
			paddedMessage.append(MESSAGE_SPACE_WIDTH - paddedMessage.size(), ' ');
		}
		else {
			paddedMessage = paddedMessage.substr(0, MESSAGE_SPACE_WIDTH);
		}

		if (pmes->self->bIsMeasuring) {
			LARGE_INTEGER CurrentTime;
			::QueryPerformanceCounter(&CurrentTime);
			LARGE_INTEGER Frequency{};
			::QueryPerformanceFrequency(&Frequency);
			LARGE_INTEGER ElapsedMicroseconds{};
			ElapsedMicroseconds.QuadPart = CurrentTime.QuadPart - pmes->self->__GapTime;
			ElapsedMicroseconds.QuadPart *= 1000000;
			ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
			double time_taken = ElapsedMicroseconds.QuadPart / 1000.0;
			pmes->self->__GapTime = CurrentTime.QuadPart;
			if (pmes->self->bIsDisplayTime) {
				std::cout << paddedMessage << " "
					<< std::fixed << std::setprecision(3) << time_taken << " msec" << std::endl;
			} else {
				std::cout << paddedMessage << std::endl;
			}
		} else {
			std::stringstream ss;
			if (pmes->self->bTrigger) {
				pmes->self->bTrigger = false;
				std::cout << paddedMessage << " "
					<< std::fixed << std::setprecision(3) << double(0) << " msec" << std::endl;
				LARGE_INTEGER li;
				::QueryPerformanceCounter(&li);
				pmes->self->__StartingTime = li.QuadPart;
				pmes->self->__GapTime = li.QuadPart;
				pmes->self->bIsMeasuring = true;

			} else{
				std::cout << paddedMessage << std::endl;
			}
		}
	}}

	, pThreadProc{ [](LPVOID pvoid)->unsigned {
		OrderedCOut* pThis = reinterpret_cast<OrderedCOut*>(pvoid);

		for (;;) {
			DWORD dw = ::WaitForSingleObjectEx(pThis->hEventThread.get(), INFINITE, TRUE);
			if (dw == WAIT_IO_COMPLETION) {
				_D("OrderedCOut APC executed.");
				continue;
			} else if (dw == WAIT_OBJECT_0) {
				_D("OrderedCOut ended.");
				return 0;
			} else {
				ENOut((pThis->__numErr = ::GetLastError()));
				return 1;
			}
		}
	} }

	, hThread{ [this](){
		HANDLE h;
		if( !(h = (HANDLE)::_beginthreadex(NULL, 0, pThreadProc, this, 0, NULL)) ){
			throw std::runtime_error(ENOut(::GetLastError()));
		};
		return h; }()
	,
		::CloseHandle
	}
{
}

OrderedCOut::~OrderedCOut() {
	::SetEvent(hEventThread.get());
	::WaitForSingleObject(hThread.get(), INFINITE);
}

OrderedCOut& OrderedCOut::Push(const std::string& str)
{
	if (__numErr)
		return *this;
	message* pmes = &(*(__mlms.Lend()) = {});
	std::copy(str.begin() , str.begin() + str.size(), pmes->buffer);
	pmes->size = str.size();
	pmes->self = this;
	if (!::QueueUserAPC(__pAPCProc, hThread.get(), (ULONG_PTR)pmes)) {
		ENOut(__numErr = ::GetLastError());
		return *this;
	}
	return *this;
}

OrderedCOut& OrderedCOut::StartTimer()
{
	if (__numErr)
		return *this;
	__PushOp(OP::START_TIMER);
	return *this;
}

bool OrderedCOut::Trigger(bool b)
{
	__PushOp(OP::START_TRIGGER);
	return bTrigger;
}

OrderedCOut& OrderedCOut::StopTimer()
{
	if (__numErr)
		return *this;
	__PushOp(OP::STOP_TIMER);
	return *this;
}

OrderedCOut& OrderedCOut::ResumeTimer(){
	if( __numErr )
		return *this;
	__PushOp(OP::RESUME_TIMER);
	return *this;
}

double OrderedCOut::TotalTime() {
	std::atomic<double> dTime{};
	::ResetEvent(hEventMessage.get());
	__PushOp(OP::TOTAL_TIME, hEventMessage.get(),&dTime);
	DWORD dw;
	if( !((dw = ::WaitForSingleObject(hEventMessage.get(), INFINITE)) == WAIT_OBJECT_0) ){
		if( dw == WAIT_FAILED ){
			ENOut(__numErr = ::GetLastError());
		}
		return 0.0;
	}
	return dTime;
}

bool OrderedCOut::ShowTimeDisplay(bool bdisp) {
	__PushOp(bdisp ? OP::START_DISPLAY_TIME : OP::STOP_DISPLAY_TIME);
	return bIsDisplayTime;
}

OrderedCOut& OrderedCOut::ResetFlag()
{
	__PushOp(OP::RESET);
	return *this;
}

OrderedCOut& OrderedCOut::MessageFlush(){
	::ResetEvent(hEventMessage.get());
	__PushOp(OP::SIG_EVENT,hEventMessage.get());
	DWORD dw;
	if(!((dw = ::WaitForSingleObject(hEventMessage.get(), INFINITE)) == WAIT_OBJECT_0)){
		if( dw == WAIT_FAILED ){
			ENOut(__numErr = ::GetLastError());
		}
		return *this;
	}
	return *this;
}

bool OrderedCOut::__PushOp(OP op, HANDLE h,void* const pvoid){
	message* pmes = &(*(__mlms.Lend()) = {});
	pmes->self = this;
	pmes->op = op;
	pmes->hEvent = h;
	pmes->pvoid = pvoid;
	if( !::QueueUserAPC(__pAPCProc, hThread.get(), (ULONG_PTR)pmes) ){
		ENOut(__numErr = ::GetLastError());
		return false;
	}
	return true;
}
