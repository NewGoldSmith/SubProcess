/**
 * @file OrderedCout.h
 * @brief OrderedCoutÉNÉâÉXêÈåæ
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#pragma once
#include <windows.h>
#include <string>
#include <sstream>
#include <iostream>
#include <exception>
#include <memory>
#include <iomanip>
#include <atomic>
#include "../Debug_fnc/debug_fnc.h"
#include "../CommonLib/MemoryLoan.h"
#include "../CommonLib/defSTRINGIZE.h"

#pragma comment(lib,  "../Debug_fnc/" STRINGIZE($CONFIGURATION) "/Debug_fnc-" STRINGIZE($CONFIGURATION) ".lib")

class OrderedCOut{
	static constexpr DWORD BUFFER_SIZE = 0x80;
	static constexpr DWORD UNIT_SIZE = 0x40;
	static constexpr DWORD CONTINUOUS_TIME_OUT = 0;
	static constexpr DWORD MESSAGE_SPACE_WIDTH = 47;
public:
	OrderedCOut();
	OrderedCOut(const OrderedCOut&) = delete;
	OrderedCOut(OrderedCOut&&) = delete;
	OrderedCOut& operator()(const OrderedCOut&) = delete;
	OrderedCOut& operator()(OrderedCOut&&) = delete;
	OrderedCOut& operator=(const OrderedCOut&) = delete;
	OrderedCOut& operator=(OrderedCOut&&) = delete;
	~OrderedCOut();
	OrderedCOut& Push(const std::string& str);
	OrderedCOut& StartTimer();
	bool Trigger(bool b);
	OrderedCOut& StopTimer();
	OrderedCOut& ResumeTimer();
	double TotalTime();
	bool ShowTimeDisplay(bool bdisp);
	OrderedCOut& ResetFlag();
	OrderedCOut& MessageFlush();
	explicit operator bool() const{ return !__numErr; }
private:
	enum OP:const DWORDLONG{
		NOOP
		, SIG_EVENT
		, START_DISPLAY_TIME
		, STOP_DISPLAY_TIME
		, START_TIMER
		, STOP_TIMER
		, RESUME_TIMER
		, RESET
		, START_TRIGGER
		, STOP_TRIGGER
		, TOTAL_TIME
	};
	struct message{
		OrderedCOut* self;
		DWORDLONG op{ OP::NOOP };
		HANDLE hEvent{};
		union{
			void* pvoid;
			size_t size;
		};
		char buffer[BUFFER_SIZE];
	};
	bool __PushOp(OP op, HANDLE h = NULL, void* const pvoid = NULL);
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> hEventThread;
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> hEventMessage;
	message __MessageArr[UNIT_SIZE];
	MemoryLoan<message> __mlms;
	VOID(CALLBACK* const __pAPCProc)(ULONG_PTR);
	LONGLONG __StartingTime{};
	LONGLONG __GapTime{};
	DWORD __numErr{};
	bool bTrigger{};
	bool bIsMeasuring{};
	bool bIsDisplayTime{ true };
	LPTHREAD_START_ROUTINE const pThreadProc;
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> hThread;

};

