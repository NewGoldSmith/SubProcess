/**
 * @file SubProcess.h
 * @brief SubProcess2作成クラス宣言
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
#include <queue>

#pragma comment(lib,  "../Debug_fnc/" STRINGIZE($CONFIGURATION) "/Debug_fnc-" STRINGIZE($CONFIGURATION) ".lib")

class  SubProcess{
	static constexpr DWORD BUFFER_SIZE_OL = 0x1000;
	static constexpr DWORD BUFER_SIZE_PIPE = 0x00;
	static constexpr DWORD UNIT_SIZE_OVERLAPPED = 0x20000;
	static constexpr DWORD DEFAULT_TIMEOUT = 100;
	static constexpr DWORD CONTINUOUS_TIMEOUT = 0;
	struct OVERLAPPED_CUSTOM;
public:
	SubProcess();
	SubProcess(const SubProcess&) = delete;
	SubProcess(SubProcess&&) = delete;
	SubProcess& operator=(const SubProcess&) = delete;
	SubProcess& operator=(SubProcess&&) = delete;
	SubProcess& operator()(const SubProcess&) = delete;
	SubProcess& operator()(SubProcess&&) = delete;
	~SubProcess(){};
	void ResetFlag()noexcept;
	DWORD SetTimeOut(DWORD uiTime)noexcept;
	bool IsTimeOut()const noexcept{ return __numErr == WAIT_TIMEOUT; };
	bool Popen(const std::string& strCommand);
	bool Pclose();
	DWORD GetExitCodeSubProcess();
	bool TerminateProcess(DWORD dw);
	bool WaitForTermination(DWORD time);
	DWORD GetLastError()const noexcept;
	explicit operator bool() const noexcept{ return !__numErr; }
	bool IsActive();
	bool IsReadable(DWORD time = DEFAULT_TIMEOUT);
	bool SetUseStdErr(bool is_use)noexcept;
	SubProcess& operator<<(const std::string& str);
	SubProcess& operator<< (std::istream& is);
	SubProcess& operator<<(std::ostream& (* const manipulator)(std::ostream&));
	SubProcess& operator<<(std::ios_base& (* const manipulator)(std::ios_base&));
	SubProcess& SleepEx(DWORD num);
	SubProcess& Flush();
	SubProcess& ClearBuffer();
	SubProcess& operator>>(std::string& str);
	SubProcess& operator>>(std::ostream& os);
	friend std::ostream& operator<<(std::ostream& os, SubProcess& sp);
	SubProcess& Await(DWORD numAwaitTime)noexcept;
	SubProcess& CErr()noexcept;
private:
	bool __ClosePipes();
	bool __CancelIo();
	bool __WriteToCli(const std::string& str);
	bool __ReadFromChild();
	bool __ReadFromChildErr();
	void __ClearQue(std::queue<OVERLAPPED_CUSTOM*>& que);
	DWORD __MoveString(std::queue<OVERLAPPED_CUSTOM*>& que, std::string& str);
	bool __TryReadOperation(DWORD timer);
	std::wstring __AtoW(const std::string& str)const;
	std::wstring __CreateNamedPipeStringW();
	std::unique_ptr< OVERLAPPED_CUSTOM, void (*)(OVERLAPPED_CUSTOM*)> __pOL;
	MemoryLoan< OVERLAPPED_CUSTOM> __mlOL;
	std::queue< OVERLAPPED_CUSTOM*> __FromChildQue;
	std::queue< OVERLAPPED_CUSTOM* > __FromChildErrQue;
	::HANDLE __hSevR{}, __hSevW{}, __hCliR{}, __hCliW{}, __hErrW{}, __hErrR{};
	enum Dir: DWORD{ COUT, CERROR };
	struct OVERLAPPED_CUSTOM{
		::OVERLAPPED ol{};
		SubProcess* self{};
		Dir dir{ Dir::COUT };
		size_t size{};
		char buffer[BUFFER_SIZE_OL]{};
	};

	std::ostringstream __ToChildBuf;
	bool __bfIsUseStdErr{};
	bool __bfIsErrOut{};
	DWORD __numErr{ 0 };
	DWORD __numAwait{ 0 };
	DWORD __numTimeOut{ DEFAULT_TIMEOUT };
	::PROCESS_INFORMATION __PI{};
	::LPOVERLAPPED_COMPLETION_ROUTINE const __pfReadFromChildCompleted;
	::LPOVERLAPPED_COMPLETION_ROUTINE const __pfWriteToChildCompleted;
};

