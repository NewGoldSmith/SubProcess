/**
 * @file SubProcess.h
 * @brief SubProcess作成クラス実装
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#include "./SubProcess.h"
SubProcess::SubProcess()noexcept:

	__mlOL(__OLArr, NUM_OVERLAPPED)

	, __pfReadFromChildCompleted{ [](
		DWORD errorCode
		, DWORD bytesTransfered
		, OVERLAPPED *overlapped) {

		std::unique_ptr<OVERLAPPED_CUSTOM ,void(*)(OVERLAPPED_CUSTOM *)> pOL = {
			[&overlapped]() {OVERLAPPED_CUSTOM *p =
			reinterpret_cast<OVERLAPPED_CUSTOM *>(overlapped);	return p; }()
			, [](OVERLAPPED_CUSTOM *p)->void {p->self->__mlOL.Return(p); } };

		if (errorCode != ERROR_SUCCESS) {
			if (errorCode != ERROR_BROKEN_PIPE) {
				debug_fnc::ENOut(errorCode);
				pOL->self->__numErr = errorCode;
			}
			return;
		}

		if (pOL->__ol.hEvent) {
			if (pOL->self->__bfIsUseStdErr)
				pOL->self->__FromChildBufErr.write(pOL->__buffer, bytesTransfered);
			else
				pOL->self->__FromChildBuf.write(pOL->__buffer, bytesTransfered);
			pOL->self->__ReadFromCliErr();
		} else {
			pOL->self->__FromChildBuf.write(pOL->__buffer, bytesTransfered);
			pOL->self->__ReadFromCli();
		}
	} }

	, __pfWriteToChildCompleted{ [](
		DWORD errorCode
		, DWORD bytesTransfered
		, OVERLAPPED *overlapped) {

		std::unique_ptr<OVERLAPPED_CUSTOM ,void(*)(OVERLAPPED_CUSTOM *)> pOL = {
			[&overlapped]() {OVERLAPPED_CUSTOM *p =
			reinterpret_cast<OVERLAPPED_CUSTOM *>(overlapped);	return p; }()
			, [](OVERLAPPED_CUSTOM *p)->void {p->self->__mlOL.Return(p); } };

		if (errorCode != ERROR_SUCCESS) {
			debug_fnc::ENOut(errorCode);
			pOL->self->__numErr = errorCode;
			return;
		}
	} }
{
	
};

bool SubProcess::Popen(const std::string &strCommand) {
	// エラーがあったか確認。
	if (__numErr)
		return false;

	// 実行中でないか確認。
	if (IsActive()) {
		__numErr = STILL_ACTIVE;
		return false;
	}

	// パイプ作成
	{
		::SECURITY_ATTRIBUTES saAttr = {};
		saAttr.nLength = sizeof(::SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = FALSE;
		saAttr.lpSecurityDescriptor = NULL;

		{
			saAttr.bInheritHandle = FALSE;
			std::wstring wstr = __CreateNamedPipeStringW();
			if ((__hSevW = ::CreateNamedPipeW(
				wstr.c_str()
				, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED
				, PIPE_TYPE_MESSAGE | PIPE_WAIT | PIPE_ACCEPT_REMOTE_CLIENTS
				, PIPE_UNLIMITED_INSTANCES
				, BUFFER_SIZE * PIPE_BUFFER_SCALE
				, BUFFER_SIZE * PIPE_BUFFER_SCALE
				, 0
				, NULL)) == INVALID_HANDLE_VALUE) {
				DWORD dw = ::GetLastError();
				debug_fnc::ENOut(dw);
				__numErr = dw;
				return FALSE;
			}

			saAttr.bInheritHandle = TRUE;
			if ((__hCliR = ::CreateFileW(
				wstr.c_str(),   // pipe name 
				PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,  // read and write access 
				FILE_SHARE_WRITE | FILE_SHARE_READ,              // no sharing 
				&saAttr,           // default security attributes
				OPEN_EXISTING,  // opens existing pipe 
				FILE_ATTRIBUTE_NORMAL,              // default attributes 
				NULL// no template file 
			)) == INVALID_HANDLE_VALUE) {
				DWORD dw = ::GetLastError();
				debug_fnc::ENOut(dw);
				__numErr = dw;
				return FALSE;
			};
		}
		{
			saAttr.bInheritHandle = FALSE;
			std::wstring wstr = __CreateNamedPipeStringW();
			if ((__hSevR = ::CreateNamedPipeW(
				wstr.c_str()
				, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED
				, PIPE_TYPE_MESSAGE | PIPE_WAIT | PIPE_ACCEPT_REMOTE_CLIENTS
				, PIPE_UNLIMITED_INSTANCES
				, BUFFER_SIZE * PIPE_BUFFER_SCALE
				, BUFFER_SIZE * PIPE_BUFFER_SCALE
				, 0
				, NULL)) == INVALID_HANDLE_VALUE) {
				DWORD dw = ::GetLastError();
				debug_fnc::ENOut(dw);
				__numErr = dw;
				return FALSE;
			}

			saAttr.bInheritHandle = TRUE;
			if ((__hCliW = ::CreateFileW(
				wstr.c_str(),   // pipe name 
				PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,  // read and write access 
				FILE_SHARE_WRITE | FILE_SHARE_READ,              // no sharing 
				&saAttr,           // default security attributes
				OPEN_EXISTING,  // opens existing pipe 
				FILE_ATTRIBUTE_NORMAL,              // default attributes 
				NULL// no template file 
			)) == INVALID_HANDLE_VALUE) {
				DWORD dw = ::GetLastError();
				debug_fnc::ENOut(dw);
				__numErr = dw;
				return FALSE;
			};
		}
		{
			saAttr.bInheritHandle = FALSE;
			std::wstring wstr = __CreateNamedPipeStringW();
			if ((__hErrR = ::CreateNamedPipeW(
				wstr.c_str()
				, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED
				, PIPE_TYPE_MESSAGE | PIPE_WAIT | PIPE_ACCEPT_REMOTE_CLIENTS
				, PIPE_UNLIMITED_INSTANCES
				, BUFFER_SIZE * PIPE_BUFFER_SCALE
				, BUFFER_SIZE * PIPE_BUFFER_SCALE
				, 0
				, NULL)) == INVALID_HANDLE_VALUE) {
				DWORD dw = ::GetLastError();
				debug_fnc::ENOut(dw);
				__numErr = dw;
				return FALSE;
			}

			saAttr.bInheritHandle = TRUE;
			if ((__hErrW = ::CreateFileW(
				wstr.c_str(),   // pipe name 
				PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,  // read and write access 
				FILE_SHARE_WRITE | FILE_SHARE_READ,              // no sharing 
				&saAttr,           // default security attributes
				OPEN_EXISTING,  // opens existing pipe 
				FILE_ATTRIBUTE_NORMAL,              // default attributes 
				NULL// no template file 
			)) == INVALID_HANDLE_VALUE) {
				DWORD dw = ::GetLastError();
				debug_fnc::ENOut(dw);
				__numErr = dw;
				return FALSE;
			};
		}
	}// パイプ作成終了

	// スタートアップインフォ設定
	::STARTUPINFO siStartInfo = {};
	siStartInfo.cb = sizeof(::STARTUPINFO);
	siStartInfo.hStdError = __hErrW;
	siStartInfo.hStdOutput = __hCliW;
	siStartInfo.hStdInput = __hCliR;
	siStartInfo.wShowWindow = SW_HIDE;
	siStartInfo.dwFlags = STARTF_USESTDHANDLES;

	std::wstring wstrPath(MAX_PATH, L'\0');
	DWORD dw;
	if (!(dw = ::GetCurrentDirectoryW(MAX_PATH, wstrPath.data()))) {
		debug_fnc::ENOut(dw);
		return false;
	}
	wstrPath.resize(dw);

	std::wstring wcmdline(__AtoW(strCommand));

	if (!::CreateProcessW(NULL,
							 wcmdline.data(),     // command line 
							 NULL,          // process security attributes 
							 NULL,          // primary thread security attributes 
							 TRUE,          // handles are inherited 
							 PROCESS_QUERY_INFORMATION,             // creation flags 
							 NULL,          // use parent's environment 
							 (wstrPath.empty() ? NULL : wstrPath.c_str()),//Exepath.parent_path().wstring().c_str(), // use parent's current directory 
							 &siStartInfo,  // STARTUPINFO pointer 
							 &__PI))  // receives PROCESS_INFORMATION
	{
		DWORD dw = ::GetLastError();
		debug_fnc::ENOut(dw);
		__numErr = dw;
		__ClosePipes();
		return FALSE;
	}

	::CloseHandle(__hCliR);
	__hCliR = NULL;
	::CloseHandle(__hCliW);
	__hCliW = NULL;
	::CloseHandle(__hErrW);
	__hErrW = NULL;

	if (!__ReadFromCli())
		return false;
	if (!__ReadFromCliErr())
		return false;
	return TRUE;
}

bool SubProcess::Pclose() {
	__CancelIo();
	__ClosePipes();
	::CloseHandle(__PI.hProcess);
	::CloseHandle(__PI.hThread);
	__PI.hProcess = NULL;
	__PI.hThread = NULL;
	return true;
}

DWORD SubProcess::GetExitCodeSubProcess() {
	DWORD dw;
	if (!::GetExitCodeProcess(__PI.hProcess, &dw)) {
		DWORD dw2 = ::GetLastError();
		debug_fnc::ENOut(dw2);
		__numErr = dw2;
		return 0;
	}
	return dw;
}

bool SubProcess::TerminateProcess(DWORD dw) {
	if (!::TerminateProcess(__PI.hProcess, dw)) {
		__numErr = ::GetLastError();
		debug_fnc::ENOut(__numErr);
		return false;
	}
	return true;
}

bool SubProcess::WaitForTermination(DWORD time) {
	DWORD dw;
	if (!((dw = ::WaitForSingleObject(__PI.hProcess, time))== WAIT_OBJECT_0)){
		if (dw == WAIT_TIMEOUT) {
			__numErr = WAIT_TIMEOUT;
			return false;
		} else {
			__numErr = dw;
			debug_fnc::ENOut(dw);
			return false;
		}
	}
	return true;
}

DWORD SubProcess::GetLastError()const noexcept {
	return __numErr;
}

SubProcess &SubProcess::operator<<(const std::string &strIn) {
	if (__numErr)
		return *this;

	if (__bfIsRaw) {
		__bfIsRaw = false;
		__ToChildBuf << strIn;
		if (!__WriteToCli(__ToChildBuf.str().c_str())) {
			return *this;
		}
		__ToChildBuf.str("");
		__ToChildBuf.clear();
		return *this;
	}

	std::string str = strIn;
	size_t start = 0;
	size_t end = str.find('\n');
	while (end != std::string::npos) {
		__ToChildBuf<< str.substr(start, end - start + 1);  // 改行を含む
		if (!__WriteToCli(__ToChildBuf.str().c_str())) {
			return *this;
		}
		__ToChildBuf.str("");
		__ToChildBuf.clear();
		start = end + 1;
		end = str.find('\n', start);
	}

	// 最後の文字が'\n'でない場合、その部分をstrに戻す
	if (!str.empty() && str.back() != '\n') {
		str = str.substr(start);
	} else {
		str.clear();
	}

	__ToChildBuf << str;
	::SleepEx(__numContinuousTimeOut, TRUE);
	return *this;
}

SubProcess &SubProcess::operator<<(std::istream &is) {
	if (__numErr)
		return *this;
	std::string line;
	std::getline(is, line);
	*this << line << std::endl;
	return *this;
}

SubProcess &SubProcess::operator<<(std::ostream &(*const manipulator)(std::ostream &)) {
	if (__numErr)
		return *this;
	if (manipulator == static_cast<std::ostream & (*)(std::ostream &)>(std::endl)) {
		// When std::endl is received, send the contents of the buffer to the client.
		__ToChildBuf << std::endl;
		if (__WriteToCli(__ToChildBuf.str())) {
			__ToChildBuf.str("");  // Clear the buffer.
			__ToChildBuf.clear();
			return *this;
		} else {
			return *this;
		}
	} else {
		// Otherwise, apply the manipulator to the buffer.
		__ToChildBuf << manipulator;
	}
	return *this;
}


bool SubProcess::__FlushWrite() {
	if (__ToChildBuf.str().size()) {
		if (!__WriteToCli(__ToChildBuf.str())) {
			return false;
		};
		__ToChildBuf.str("");  // Clear the buffer.
	}
	__ToChildBuf.clear();
	return true;
}

SubProcess &SubProcess::operator<<(std::ios_base &(*const manipulator)(std::ios_base &)) {
	if (__numErr)
		return *this;
	__ToChildBuf << manipulator;
	::SleepEx(__numContinuousTimeOut, TRUE);
	return *this;
}

SubProcess &SubProcess::SleepEx(DWORD num) {
	if (__numErr)
		return *this;
	::SleepEx(num, TRUE);
	return *this;
}

SubProcess &SubProcess::Flush() {
	if (__numErr)
		return *this;
	__FlushWrite();
	//__FlushRead();
	return *this;
}

SubProcess &SubProcess::ClearBuffer() {
	if (__numErr)
		return *this;
	__ToChildBuf.str("");
	__ToChildBuf.clear();
	__FromChildBuf.str("");
	__FromChildBuf.clear();
	__FromChildBufErr.str("");
	__FromChildBufErr.clear();
	return *this;
}

SubProcess &SubProcess::operator>>(std::string &str) {
	str.clear();

	if (__numErr)
		return *this;

	DWORD timeout;
	if (__numAwait) {
		timeout = __numAwait;
		__numAwait = 0;
	} else {
		timeout = __numTimeOut;
	}

	if (__bfIsErrOut) {
		__bfIsErrOut = false;
		if (__FromChildBufErr.str().empty()) {
			if (!__StrongReadFromCli(timeout)) {
				return *this;
			}
		}
		str = move(__FromChildBufErr.str());
		__FromChildBufErr.str("");
		__FromChildBufErr.clear();
	} else {
		if (__FromChildBuf.str().empty()) {
			if (!__StrongReadFromCli(timeout)) {
				return *this;
			}
		}
		str = move(__FromChildBuf.str());
		__FromChildBuf.str("");
		__FromChildBuf.clear();
	}
	return *this;
}

SubProcess &SubProcess::operator>>(std::ostream &os) {
	if (__numErr)
		return *this;
	std::string str;
	operator >> (str);
	os << str;
	return *this;
}

SubProcess &SubProcess::Await(DWORD numAwaitTime)noexcept {
	__numAwait = numAwaitTime;
	return *this;
}

SubProcess &SubProcess::CErr()noexcept {
	__bfIsErrOut = true;
	return *this;
}

SubProcess &SubProcess::Raw() noexcept {
	__bfIsRaw = true;
	return *this;
}

bool SubProcess::IsActive(){
	if (!__PI.hProcess)
		return false;
	DWORD dw;
	if (!::GetExitCodeProcess(__PI.hProcess,&dw)) {
		DWORD dw2 = ::GetLastError();
		debug_fnc::ENOut(dw2);
		__numErr = dw2;
		return false;
	}
	return dw == STILL_ACTIVE ? true : false;
}

bool SubProcess::IsReadable(){
	if (__numErr)
		return false;

	::SleepEx(__numContinuousTimeOut, TRUE);
	if (__bfIsErrOut) {
		__bfIsErrOut = false;
		if (__FromChildBufErr.str().size()) {
			return true;
		}
	} else {
		if (__FromChildBuf.str().size()) {
			return true;
		}
	}
	return false;
}

bool SubProcess::SetUseStdErr(bool is_use)noexcept {
	bool tmp = __bfIsUseStdErr;
	__bfIsUseStdErr = is_use;
	return tmp;
}

inline std::wstring SubProcess::__AtoW(const std::string &str)const{
	if (str.empty()) {
		return std::wstring();
	}
	std::wstring wstr(str.size(), L'\0');
	//ANSIからユニコードへ変換
	int size(0);
	if (!(size = ::MultiByteToWideChar(
		CP_ACP
		, 0
		, str.data()
		, static_cast<int>(str.size())
		, wstr.data()
		, static_cast<int>(wstr.size())))) {
		debug_fnc::EOut;
		wstr.resize(0);
		return wstr;
	}
	wstr.resize(size);
	return wstr;
}

std::string SubProcess::__GetParentPathA() {
	std::string str(MAX_PATH,'\0');
	DWORD dw;
	if (!(dw = ::GetCurrentDirectoryA(MAX_PATH, str.data()))) {
		debug_fnc::EOut;
		return std::string("");
	}
	str.resize(dw);
	return str;
}

bool SubProcess::__ReadFromCli() {
	OVERLAPPED_CUSTOM *pOL = &(*(__mlOL.Lend()) = {});
	pOL->self = this;
	DWORD dw;
	if (!::ReadFileEx(
		__hSevR
		, &(pOL->__buffer)
		, sizeof(OVERLAPPED_CUSTOM::__buffer)
		, (::OVERLAPPED *)pOL
		, __pfReadFromChildCompleted)) {
		dw = ::GetLastError();
		if (dw == WAIT_IO_COMPLETION) {
			::SleepEx(__numContinuousTimeOut, TRUE);
			return TRUE;
		} else {
			debug_fnc::ENOut(dw);
			return FALSE;
		}
	}
	return true;
}

bool SubProcess::__ReadFromCliErr() {
	OVERLAPPED_CUSTOM *pOL = &(*(__mlOL.Lend()) = { .__ol{.hEvent = __hErrW} });

	pOL->self = this;
	DWORD dw;
	if (!::ReadFileEx(
		__hErrR
		, &(pOL->__buffer)
		, sizeof(OVERLAPPED_CUSTOM::__buffer)
		, (OVERLAPPED *)pOL
		, __pfReadFromChildCompleted)) {
		dw = ::GetLastError();
		if (dw == WAIT_IO_COMPLETION) {
			::SleepEx(__numContinuousTimeOut, TRUE);
			return TRUE;
		} else {
			debug_fnc::ENOut(dw);
			return FALSE;
		}
	}
	return true;
}

bool SubProcess::__StrongReadFromCli(DWORD timeout) {
	DWORD dw;
	if (!(dw = ::SleepEx(timeout, TRUE))) {
		// タイムアウトの場合
		__numErr = WAIT_TIMEOUT;
		return false;
	} else if (dw == WAIT_IO_COMPLETION) {
		for (;;) {
			if (DWORD dw2 = ::SleepEx(__numContinuousTimeOut, TRUE)) {
				if (dw2 == WAIT_IO_COMPLETION) {
					continue;
				} else {
					// WAIT_IO_COMPLETION以外のコードの場合
					debug_fnc::ENOut(dw2);
					__numErr = dw2;
					return false;
				}
			} else {
				// ２回目以降のタイムアウトの場合
				return true;
			}
		}
	} else {
		// 1回目のWAIT_IO_COMPLETION以外のコードの場合
		__numErr = dw;
		debug_fnc::ENOut(dw);
		return false;
	}
	return true;
}

std::wstring SubProcess::__CreateNamedPipeStringW() {
	GUID guid;
	std::wstring wstr(0x100, L'\0');
	if (CoCreateGuid(&guid) != S_OK) {
		_MES("CoCreateGuid Err.");
		throw std::runtime_error("CoCreateGuid Err.");
	};
	wstr.resize(StringFromGUID2(guid, wstr.data(), (int)wstr.capacity()));
	if (!wstr.size()) {
		_MES("StringFromGUID2 Err.");
		throw std::runtime_error("StringFromGUID2 Err.");
	}
	wstr = L"\\\\.\\pipe\\" + __AtoW(__FILE__) + wstr + L"\\";
	return wstr;
}

bool SubProcess::__ClosePipes() {
	::CloseHandle(__hCliR);
	__hCliR = NULL;
	::CloseHandle(__hCliW);
	__hCliW = NULL;
	::CloseHandle(__hSevR);
	__hSevR = NULL;
	::CloseHandle(__hSevW);
	__hSevW = NULL;
	::CloseHandle(__hErrR);
	__hErrR = NULL;
	::CloseHandle(__hErrW);
	__hErrW = NULL;
	return true;
}

bool SubProcess::__CancelIo() {
	if (!::CancelIoEx(__hSevR, NULL)) {
		DWORD dw = ::GetLastError();
		debug_fnc::ENOut(dw);
		__numErr = dw;
	}
	::SleepEx(__numContinuousTimeOut, TRUE);
	if (! ::CancelIoEx(__hSevW, NULL)) {
		DWORD dw = ::GetLastError();
		debug_fnc::ENOut(dw);
		__numErr = dw;
	}
	::SleepEx(__numContinuousTimeOut, TRUE);
	if (! ::CancelIoEx(__hErrR, NULL)) {
		DWORD dw = ::GetLastError();
		debug_fnc::ENOut(dw);
		__numErr = dw;
	}
	::SleepEx(__numContinuousTimeOut, TRUE);

	return true;
}

void SubProcess::ResetFlag()noexcept {
	__numErr = 0;
}

DWORD SubProcess::SetTimeOut(DWORD uiTime)noexcept {
	DWORD tmp = __numTimeOut;
	__numTimeOut = uiTime;
	return tmp;
}

bool SubProcess::__WriteToCli(const std::string &str) {
	size_t pos = 0;
	while (pos < str.size()) {
		size_t len = std::min<size_t>(str.size() - pos, static_cast<size_t>(BUFFER_SIZE));
		std::string chunk = str.substr(pos, len);
		pos += len;

		OVERLAPPED_CUSTOM *pOL = &(*(__mlOL.Lend()) = {});
		std::copy(chunk.begin(), chunk.end(), pOL->__buffer);
		pOL->self = this;
		DWORD dw;
		if (!::WriteFileEx(
			__hSevW
			, &(pOL->__buffer)
			, (DWORD)chunk.size()
			, (::OVERLAPPED *)pOL
			, __pfWriteToChildCompleted)) {
			dw = ::GetLastError();
			if (dw == WAIT_IO_COMPLETION) {
				::SleepEx(__numContinuousTimeOut, TRUE);
				continue;
			} else {
				__numErr = dw;
				debug_fnc::ENOut(dw);
				return FALSE;
			}
		}
		::SleepEx(__numContinuousTimeOut, TRUE);
	}
	return true;
}

std::ostream &operator<<(std::ostream &os, SubProcess &sp) {
	std::string str;
	sp >> str;
	os << str;
	return os;
}
