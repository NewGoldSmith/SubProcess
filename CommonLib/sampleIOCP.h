/**
 * @file sampleIOCP.h
 * @brief sampleIOCPçÏê¨ÉNÉâÉXêÈåæ
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#pragma once
#include <Windows.h>
#include <string>
#include <sstream>
#include <iostream>
#include <exception>
#include <thread>
#include <string.h>
#include <algorithm>
#include "../Debug_fnc/debug_fnc.h"
#include "defSTRINGIZE.h"
#include "MemoryLoan.h"
#include "../CommonLib/OrderedCout.h"
#pragma comment(lib,  "../Debug_fnc/" STRINGIZE($CONFIGURATION) "/Debug_fnc-" STRINGIZE($CONFIGURATION) ".lib")

class  sampleIOCP {
	static constexpr DWORD BUFFER_SIZE_OL = 0x10;
	static constexpr DWORD BUFFER_SIZE_CL_SIDE = 0x400;
	static constexpr DWORD BUFER_SIZE_PIPE = 0x100;
	static constexpr DWORD NUM_OVERLAPPED = 0x10;
	static constexpr DWORD DEFAULT_TIMEOUT = 2000;
	static constexpr DWORD CONTINUOUS_TIMEOUT = 0;
	static constexpr DWORD CLIENT_WORK_TIME = 1;
	static constexpr DWORD MAX_STRING = 0x100;
public:
	sampleIOCP();
	sampleIOCP(const sampleIOCP&) = delete;
	sampleIOCP& operator=(const sampleIOCP&) = delete;
	sampleIOCP& operator()(const sampleIOCP&) = delete;
	sampleIOCP(sampleIOCP&&)noexcept = delete;
	sampleIOCP& operator=(sampleIOCP&&)noexcept = delete;
	sampleIOCP& operator()(sampleIOCP&&)noexcept = delete;
	~sampleIOCP();
	std::wstring CreateNamedPipeString();
	bool CreatePipes();
	bool ClosePipes();
	void ResetFlag();
	unsigned SetTimeOut(unsigned uiTime);
	bool IsTimeOut() { return __numErr == ERROR_TIMEOUT; };
	sampleIOCP &operator<<(const std::string &str);
	sampleIOCP &operator>>(std::string &str);
	explicit operator bool() const { return !__numErr; }
private:
	bool __WriteToCli(const std::string &str);
	bool __ReadFromCli();
	bool __WriteCliSide(const std::string &str);
	bool __ReadCliSide(std::string &str);
	bool __StartCliThread();
	bool __EndCliThread();
	struct OVERLAPPED_CUSTOM {
		OVERLAPPED ol{};
		sampleIOCP* self{};
		char buffer[BUFFER_SIZE_OL]{};
	};
	OVERLAPPED_CUSTOM OLArr[NUM_OVERLAPPED];
	HANDLE __hSevR{}, __hSevW{}, __hCliR{}, __hCliW{};
	DWORD __numErr{};
	int __TimeOut{ DEFAULT_TIMEOUT };
	std::stringstream __ssFromCli;
	MemoryLoan< OVERLAPPED_CUSTOM> __mlOL;
	LPTHREAD_START_ROUTINE const pThreadCli;
	LPOVERLAPPED_COMPLETION_ROUTINE const pfReadFromCliCompleted;
	LPOVERLAPPED_COMPLETION_ROUTINE const pfWriteToCliCompleted;
	HANDLE __hThreadCli;
public:
	OrderedCOut OrCout;
};
