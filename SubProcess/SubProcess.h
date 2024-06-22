/**
 * @file SubProcess.h
 * @brief SubProcess作成クラス宣言
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#pragma once

#include <Windows.h>
#include <string>
#include <iomanip>
#include <iosfwd>
#include <xiosbase>
#include "../CommonLib/defSTRINGIZE.h"
#include "../CommonLib/MemoryLoan.h"
#include "../Debug_fnc/debug_fnc.h"

#pragma comment(lib,  "../Debug_fnc/" STRINGIZE($CONFIGURATION) "/Debug_fnc-" STRINGIZE($CONFIGURATION) ".lib")

class  SubProcess {
	static constexpr int BUFFER_SIZE = 0x400;
	static constexpr int PIPE_BUFFER_SCALE = 1;
	static constexpr int NUM_OVERLAPPED = 0x8;
	static constexpr int DEFAULT_TIMEOUT = 1000;
	static constexpr int CONTINUOUS_TIMEOUT = 1;
	struct OVERLAPPED_CUSTOM;
public:
	SubProcess()noexcept;
	SubProcess(SubProcess &) = delete;
	SubProcess(SubProcess &&) = delete;
	SubProcess &operator=(SubProcess &) = delete;
	SubProcess &operator=(SubProcess &&) = delete;
	SubProcess &operator()(SubProcess &) = delete;
	SubProcess &operator()(SubProcess &&) = delete;
	~SubProcess() {};
	void ResetFlag()noexcept;
	DWORD SetTimeOut(DWORD uiTime)noexcept;
	bool IsTimeOut()const noexcept { return __numErr == WAIT_TIMEOUT; };
	bool Popen(const std::string &strCommand);
	bool Pclose();
	DWORD GetExitCodeSubProcess();
	bool TerminateProcess(DWORD dw);
	bool WaitForTermination(DWORD time);
	DWORD GetLastError()const noexcept;
	explicit operator bool() const noexcept { return !__numErr; }
	bool IsActive();
	bool IsReadable();
	bool SetUseStdErr(bool is_use)noexcept;
	SubProcess &operator<<(const std::string &str);
	SubProcess &operator<< (std::istream &is);
	SubProcess &operator<<(std::ostream &(*const manipulator)(std::ostream &));
	SubProcess &operator<<(std::ios_base &(*const manipulator)(std::ios_base &));
	SubProcess &SleepEx(DWORD num);
	SubProcess &Flush();
	SubProcess &ClearBuffer();
	SubProcess &operator>>(std::string &str);
	SubProcess &operator>>(std::ostream &os);
	friend std::ostream &operator<<(std::ostream &os, SubProcess &sp);
	SubProcess &Await(DWORD numAwaitTime)noexcept;
	SubProcess &CErr()noexcept;
	SubProcess &Raw()noexcept;
private:
	bool __ClosePipes();
	bool __FlushWrite();
	bool __CancelIo();
	bool __WriteToCli(const std::string &str);
	bool __ReadFromCli();
	bool __ReadFromCliErr();
	bool __StrongReadFromCli(DWORD timeout= DEFAULT_TIMEOUT);
	std::wstring __AtoW(const std::string &str)const;
	std::string __GetParentPathA();
	std::wstring __CreateNamedPipeStringW();
	struct OVERLAPPED_CUSTOM {
		::OVERLAPPED ol{};
		SubProcess *self{};
		char buffer[BUFFER_SIZE]{};
	};

	OVERLAPPED_CUSTOM __OLArr[NUM_OVERLAPPED];
	MemoryLoan< OVERLAPPED_CUSTOM> __mlOL;
	bool __bfIsUseStdErr{};
	bool __bfIsErrOut{};
	bool __bfIsRaw{};
	::HANDLE __hSevR{}, __hSevW{}, __hCliR{}, __hCliW{}, __hErrW{}, __hErrR{};
	DWORD __numErr{ 0 };
	DWORD __numAwait{ 0 };
	DWORD __numTimeOut{ DEFAULT_TIMEOUT };
	DWORD __numContinuousTimeOut{ CONTINUOUS_TIMEOUT };
	::PROCESS_INFORMATION __PI{};
	::LPOVERLAPPED_COMPLETION_ROUTINE const __pfReadFromChildCompleted;
	::LPOVERLAPPED_COMPLETION_ROUTINE const __pfWriteToChildCompleted;
	std::ostringstream __ToChildBuf;
	std::stringstream __FromChildBuf;
	std::stringstream __FromChildBufErr;
};

