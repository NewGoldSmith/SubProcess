/**
 * @file OrderedCout.cpp
 * @brief OrderedCoutクラス実装
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#include "OrderedCout.h"

OrderedCOut::OrderedCOut() :
	hEvent{ []() {HANDLE h;
		if (!(h = ::CreateEvent(NULL,TRUE,FALSE,NULL))) {
			std::string str = debug_fnc::ENOut(::GetLastError());
			throw std::runtime_error(str);
		};
		return h; }()
	,
		::CloseHandle }
	
	,__mlms(__MessageArr, UNIT_SIZE)

	, __pAPCProc{ [](ULONG_PTR dwParam) {
		std::unique_ptr<message ,void(*)(message*)> pmes = {
			reinterpret_cast<message*>(dwParam)
			, [](message* p)->void {p->self->__mlms.Return(p); } };

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

		if (pmes->self->__StartingTime.QuadPart) {
			LARGE_INTEGER CurrentTime;
			::QueryPerformanceCounter(&CurrentTime);
			LARGE_INTEGER Frequency{};
			::QueryPerformanceFrequency(&Frequency);
			LARGE_INTEGER ElapsedMicroseconds{};
			ElapsedMicroseconds.QuadPart = CurrentTime.QuadPart - pmes->self->__GapTime.QuadPart;
			ElapsedMicroseconds.QuadPart *= 1000000;
			ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
			double time_taken = ElapsedMicroseconds.QuadPart / 1000.0;
			pmes->self->__GapTime.QuadPart = CurrentTime.QuadPart;

			std::cout << paddedMessage << " "
				<< std::fixed << std::setprecision(3) << time_taken << " msec" << std::endl;
		} else {
			std::stringstream ss;
			if (pmes->self->bTrigger) {
				std::cout << paddedMessage << " "
					<< std::fixed << std::setprecision(3) << double(0) << " msec" << std::endl;
				pmes->self->StartTimer();
			} else {
				std::cout << paddedMessage << std::endl;
			}
		}
	}}

	, pThreadProc{ [](LPVOID pvoid)->DWORD {
		OrderedCOut* pThis = reinterpret_cast<OrderedCOut*>(pvoid);

		for (;;) {
			DWORD dwWaitResult = ::WaitForSingleObjectEx(pThis->hEvent.get(), INFINITE, TRUE);
			if (dwWaitResult == WAIT_IO_COMPLETION) {
				_D("APC executed.");
			} else if (dwWaitResult == WAIT_OBJECT_0) {
				_D("Event signaled.");
				return 0;
			} else {
				debug_fnc::ENMOut((pThis->__numErr = ::GetLastError()), "Wait failed.");
				return 1;
			}
		}
	} }

	, hThread{ [this]() {
		HANDLE h;
		if (!(h = ::CreateThread(NULL,0,pThreadProc,this,0,NULL))) {
			std::string str = debug_fnc::ENOut(::GetLastError());
			throw std::runtime_error(str);
		};
		return h; }()
	,
		::CloseHandle
	}
{
}

OrderedCOut::~OrderedCOut() {
	::SetEvent(hEvent.get());
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
		debug_fnc::ENOut(__numErr = ::GetLastError());
		return *this;
	}
	return *this;
}

OrderedCOut& OrderedCOut::StartTimer()
{
	if (__numErr)
		return *this;
	::QueryPerformanceCounter(&__StartingTime);
	__GapTime.QuadPart = __StartingTime.QuadPart;
	return *this;
}

bool OrderedCOut::Trigger(bool b)
{
	bool tmp = bTrigger;
	bTrigger = b;
	return tmp;
}

OrderedCOut& OrderedCOut::StopTimer()
{
	if (__numErr)
		return *this;
	__StartingTime = {};
	return *this;
}

OrderedCOut& OrderedCOut::ResetFlag()
{
	__numErr = 0;
	bTrigger = false;
	__StartingTime = {};
	return *this;
}
