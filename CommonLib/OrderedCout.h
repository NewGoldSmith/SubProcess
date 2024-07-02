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
#include "../Debug_fnc/debug_fnc.h"
#include "../CommonLib/MemoryLoan.h"
#include "../CommonLib/defSTRINGIZE.h"

#pragma comment(lib,  "../Debug_fnc/" STRINGIZE($CONFIGURATION) "/Debug_fnc-" STRINGIZE($CONFIGURATION) ".lib")

class OrderedCOut
{
	static constexpr DWORD BUFFER_SIZE = 0x100;
	static constexpr DWORD UNIT_SIZE = 0x10;
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
	OrderedCOut& ResetFlag();
	explicit operator bool() const { return !__numErr; }
private:
	struct message {
		OrderedCOut* self;
		size_t size;
		char buffer[BUFFER_SIZE];
	};
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> hEvent;
	MemoryLoan<message> __mlms;
	VOID(CALLBACK* const __pAPCProc)(ULONG_PTR);
	LARGE_INTEGER __StartingTime{};
	LARGE_INTEGER __GapTime{};
	DWORD __numErr{};
	bool bTrigger{};
	message __MessageArr[UNIT_SIZE];
	LPTHREAD_START_ROUTINE const pThreadProc;
	std::unique_ptr<std::remove_pointer_t<HANDLE>,decltype(CloseHandle)*> hThread;

};

