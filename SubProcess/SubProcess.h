/**
 * @file SubProcess.h
 * @brief SubProcess作成クラス宣言
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#pragma once
#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#include <Windows.h>
#include <string>
#include <iomanip>
#include <iosfwd>
#include <xiosbase>

#include "../Debug_fnc/debug_fnc.h"
#include "./MemoryLoan.h"
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
	bool SetUseStdErr(bool is_use);
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
	SubProcess &Await(DWORD numAwaitTime);
	SubProcess &CErr();
private:
	bool __ClosePipes();
	bool __FlushWrite();
	bool __FlushRead();
	bool __CancelIo();
	bool __WriteToCli(const std::string &str);
	bool __ReadFromCli();
	bool __ReadFromCliErr();
	bool __StrongReadFromCli(DWORD timeout= DEFAULT_TIMEOUT);
	std::wstring __AtoW(const std::string &str)const;
	std::string __GetParentPathA();
	std::wstring __CreateNamedPipeStringW();
	struct OVERLAPPED_CUSTOM {
		::OVERLAPPED __ol{};
		SubProcess *self{};
		char __buffer[BUFFER_SIZE]{};
	};

	OVERLAPPED_CUSTOM __OLArr[NUM_OVERLAPPED];
	MemoryLoan< OVERLAPPED_CUSTOM> __mlOL;
	bool __bfIsUseStdErr{};
	bool __bfIsErrOut{};
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
/**
 * @section 使用例
 *
 * @code{ .cpp }
// test2.cpp
#include <crtdbg.h>
#include "./SubProcess.h"
using namespace std;

int main() {
	{
		string str;
		SubProcess sp;
		if (!sp.Popen(R"(cmd.exe)"))
			return 1;
		if (!(sp.Await(3000) >> cout))
			return 1;

		sp.SetUseStdErr(true);

		if (!(sp >> cout)) {
			DWORD result = sp.GetLastError();
			cout << "\n\nError code is " << result << endl;
			sp.ResetFlag();
		}

		if (!(sp << "chcp" << endl))
			return 1;
		if (!(sp >> str))
			return 1;
		cout << str;

		if (!(sp << "chcp" << "\n"))
			return 1;
		for (; sp.IsReadable();){
			sp >> cout;
		}

		if (!(sp << "chcp"))
			return 1;
		if (sp.IsReadable()) {
			if (!(sp >> cout))
				return 1;
		}
		if (!sp.Flush())
			return 1;
		if (sp.IsReadable()) {
			if (!(sp >> cout))
				return 1;
		}
		if (!(sp << endl))
			return 1;
		if (sp.IsReadable()) {
			if (!(sp >> cout))
				return 1;
		}

		if (!(sp << "chcp " ))
			return 1;
		cout << "\n\nPlease enter the code page number." << endl;
		if (!(sp << cin))
			return 1;
		if (!(sp >> cout))
			return 1;
		if (sp.CErr().IsReadable()) {
			if (!(sp.CErr() >> cerr))
				return 1;
		}

		if (!(sp << "exit" << endl))
			return 1;
		for (; sp.IsReadable();) {
			sp.Await(1000) >> cout;
			if (!sp.SleepEx(100))
				return 1;
		}

		if (!(sp.WaitForTermination(INFINITE)))
			return 1;

		DWORD dw = sp.GetExitCodeSubProcess();
		cout << "\nExit code is " << dw << endl;

		sp.Pclose();

		cout << "\nThe SubProcess demo has successfully concluded." << endl;
	}
	_CrtDumpMemoryLeaks();
}
 * @endcode
 */
};

