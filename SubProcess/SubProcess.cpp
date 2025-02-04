﻿/**
 * @file SubProcess.h
 * @brief SubProcess2作成クラス実装
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#include "SubProcess.h"
SubProcess::SubProcess():

	__pOL{ [](){OVERLAPPED_CUSTOM* p =
	new OVERLAPPED_CUSTOM[UNIT_SIZE_OVERLAPPED]; return p; }()
	, [](OVERLAPPED_CUSTOM* p){delete[]p; } }

	, __mlOL(__pOL.get(), UNIT_SIZE_OVERLAPPED)

	, __pfReadFromChildCompleted{ [](
		DWORD errorCode
		, DWORD bytesTransferred
		, OVERLAPPED* overlapped){

		OVERLAPPED_CUSTOM* pOL = reinterpret_cast<OVERLAPPED_CUSTOM*>(overlapped);

		if( errorCode != ERROR_SUCCESS ){
			ENOut(pOL->self->__numErr = errorCode);
		}

		pOL->size = bytesTransferred;

		if( pOL->dir == Dir::COUT ){
			pOL->self->__FromChildQue.push(pOL);
			pOL->self->__ReadFromChild();
		} else{
			pOL->self->__FromChildErrQue.push(pOL);
			pOL->self->__ReadFromChildErr();
		}
	} }

	, __pfWriteToChildCompleted{ [](
		DWORD errorCode
		, DWORD bytesTransferred
		, OVERLAPPED* overlapped){

		std::unique_ptr<OVERLAPPED_CUSTOM ,void(*)(OVERLAPPED_CUSTOM*)> pOL = {
			reinterpret_cast<OVERLAPPED_CUSTOM*>(overlapped)
			, [](OVERLAPPED_CUSTOM* p)->void{p->self->__mlOL.Return(p); } };

		if( errorCode != ERROR_SUCCESS ){
			ENOut(pOL->self->__numErr = errorCode);
		}
	} }{

};

bool SubProcess::Popen(const std::string& strCommand){
	// エラーがあったか確認。
	if( __numErr )
		return false;

	// 実行中でないか確認。
	if( IsActive() ){
		__numErr = STILL_ACTIVE;
		return false;
	}

	// パイプ作成
	{
		SECURITY_ATTRIBUTES saAttr = {};
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		{
			std::wstring wstr = __CreateNamedPipeStringW();
			if( (__hSevW = ::CreateNamedPipeW(
				wstr.c_str()
				, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED
				, PIPE_TYPE_BYTE | PIPE_WAIT | PIPE_ACCEPT_REMOTE_CLIENTS
				, PIPE_UNLIMITED_INSTANCES
				, BUFER_SIZE_PIPE
				, BUFER_SIZE_PIPE
				, 0
				, NULL)) == INVALID_HANDLE_VALUE ){
				ENOut(__numErr = ::GetLastError());
				return FALSE;
			}

			if( (__hCliR = ::CreateFileW(
				wstr.c_str(),   // pipe name 
				PIPE_ACCESS_DUPLEX,  // read and write access 
				FILE_SHARE_WRITE | FILE_SHARE_READ,              // no sharing 
				&saAttr,           // default security attributes
				OPEN_EXISTING,  // opens existing pipe 
				FILE_ATTRIBUTE_NORMAL,              // default attributes 
				NULL// no template file 
			)) == INVALID_HANDLE_VALUE ){
				ENOut(__numErr = ::GetLastError());
				return FALSE;
			};
		}
		{
			std::wstring wstr = __CreateNamedPipeStringW();
			if( (__hSevR = ::CreateNamedPipeW(
				wstr.c_str()
				, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED
				, PIPE_TYPE_BYTE | PIPE_WAIT | PIPE_ACCEPT_REMOTE_CLIENTS
				, PIPE_UNLIMITED_INSTANCES
				, BUFER_SIZE_PIPE
				, BUFER_SIZE_PIPE
				, 0
				, NULL)) == INVALID_HANDLE_VALUE ){
				ENOut(__numErr = ::GetLastError());
				return FALSE;
			}

			if( (__hCliW = ::CreateFileW(
				wstr.c_str(),
				PIPE_ACCESS_DUPLEX,
				FILE_SHARE_WRITE | FILE_SHARE_READ,
				&saAttr,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL
			)) == INVALID_HANDLE_VALUE ){
				ENOut(__numErr = ::GetLastError());
				return FALSE;
			};
		}
		{
			std::wstring wstr = __CreateNamedPipeStringW();
			if( (__hErrR = ::CreateNamedPipeW(
				wstr.c_str()
				, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED
				, PIPE_TYPE_BYTE | PIPE_WAIT | PIPE_ACCEPT_REMOTE_CLIENTS
				, PIPE_UNLIMITED_INSTANCES
				, BUFER_SIZE_PIPE
				, BUFER_SIZE_PIPE
				, 0
				, NULL)) == INVALID_HANDLE_VALUE ){
				ENOut(__numErr = ::GetLastError());
				return FALSE;
			}

			if( (__hErrW = ::CreateFileW(
				wstr.c_str(),   // pipe name 
				PIPE_ACCESS_DUPLEX,  // read and write access 
				FILE_SHARE_WRITE | FILE_SHARE_READ,              // no sharing 
				&saAttr,           // default security attributes
				OPEN_EXISTING,  // opens existing pipe 
				FILE_ATTRIBUTE_NORMAL,              // default attributes 
				NULL// no template file 
			)) == INVALID_HANDLE_VALUE ){
				ENOut(__numErr = ::GetLastError());
				return FALSE;
			};
		}
	}// パイプ作成終了

	// スタートアップインフォ設定
	::STARTUPINFO siStartInfo = {};
	siStartInfo.cb = sizeof(::STARTUPINFO);
	siStartInfo.hStdError = __bfIsUseStdErr ? __hErrW : __hCliW;
	siStartInfo.hStdOutput = __hCliW;
	siStartInfo.hStdInput = __hCliR;
	siStartInfo.wShowWindow = SW_HIDE;
	siStartInfo.dwFlags = STARTF_USESTDHANDLES;

	std::wstring wstrPath(MAX_PATH, L'\0');
	wstrPath.resize(::GetCurrentDirectoryW(MAX_PATH, wstrPath.data()));
	if( !wstrPath.size() ){
		ENOut(__numErr = ::GetLastError());
		return false;
	}

	std::wstring wcmdline(__AtoW(strCommand));

	if( !::CreateProcessW(NULL,
		wcmdline.data(),     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		PROCESS_QUERY_INFORMATION,// creation flags 
		NULL,          // use parent's environment 
		wstrPath.empty() ? NULL : wstrPath.c_str(),// use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&__PI) )  // receives PROCESS_INFORMATION
	{
		ENOut(__numErr = ::GetLastError());
		__ClosePipes();
		return FALSE;
	}

	::CloseHandle(__hCliR);
	__hCliR = NULL;
	::CloseHandle(__hCliW);
	__hCliW = NULL;
	::CloseHandle(__hErrW);
	__hErrW = NULL;

	if( !__ReadFromChild() )
		return false;
	if( __bfIsUseStdErr )
		if( !__ReadFromChildErr() )
			return false;

	return TRUE;
}

bool SubProcess::Pclose(){
	__CancelIo();
	__TryReadOperation(DEFAULT_TIMEOUT);
	__ClosePipes();
	::WaitForSingleObject(__PI.hProcess, INFINITE);
	::CloseHandle(__PI.hProcess);
	::CloseHandle(__PI.hThread);
	__PI.hProcess = NULL;
	__PI.hThread = NULL;
	ClearBuffer();
	return true;
}

DWORD SubProcess::GetExitCodeSubProcess(){
	DWORD dw;
	if( !::GetExitCodeProcess(__PI.hProcess, &dw) ){
		ENOut(__numErr = ::GetLastError());
		return 0;
	}
	return dw;
}

bool SubProcess::TerminateProcess(DWORD dw){
	if( !::TerminateProcess(__PI.hProcess, dw) ){
		ENOut(__numErr = ::GetLastError());
		return false;
	}
	return true;
}

bool SubProcess::WaitForTermination(DWORD time){
	if( !((__numErr = ::WaitForSingleObject(__PI.hProcess, time)) == WAIT_OBJECT_0) ){
		if( __numErr == WAIT_FAILED ){
			ENOut(__numErr = ::GetLastError());
			return false;
		} else{
			ENOut(__numErr);
			return false;
		}
	}
	return true;
}

DWORD SubProcess::GetLastError()const noexcept{
	return __numErr;
}

SubProcess& SubProcess::operator<<(const std::string& strIn){
	if( __numErr )
		return *this;

	std::string str = strIn;
	size_t start = 0;
	size_t end = str.find('\n');
	while( end != std::string::npos ){
		__ToChildBuf << str.substr(start, end - start + 1);  // 改行を含む
		if( !__WriteToCli(__ToChildBuf.str().c_str()) ){
			return *this;
		}
		__ToChildBuf.str("");
		__ToChildBuf.clear();
		start = end + 1;
		end = str.find('\n', start);
	}

	// 最後の文字が'\n'でない場合、その部分をstrに戻す
	if( !str.empty() && str.back() != '\n' ){
		str = str.substr(start);
	} else{
		str.clear();
	}

	__ToChildBuf << str;
	return *this;
}

SubProcess& SubProcess::operator<<(std::istream& is){
	if( __numErr )
		return *this;
	std::string line;
	std::getline(is, line);
	*this << line << std::endl;
	return *this;
}

SubProcess& SubProcess::operator<<(std::ostream& (* const manipulator)(std::ostream&)){
	if( __numErr )
		return *this;
	if( manipulator == static_cast<std::ostream & (*)(std::ostream&)>(std::endl) ){
		// When std::endl is received, send the contents of the buffer to the client.
		__ToChildBuf << std::endl;
		if( __WriteToCli(__ToChildBuf.str()) ){
			__ToChildBuf.str("");  // Clear the buffer.
			__ToChildBuf.clear();
			return *this;
		} else{
			return *this;
		}
	} else{
		// Otherwise, apply the manipulator to the buffer.
		__ToChildBuf << manipulator;
	}
	return *this;
}

SubProcess& SubProcess::operator<<(std::ios_base& (* const manipulator)(std::ios_base&)){
	if( __numErr )
		return *this;
	__ToChildBuf << manipulator;
	__TryReadOperation(CONTINUOUS_TIMEOUT);
	return *this;
}

SubProcess& SubProcess::SleepEx(DWORD num){
	if( __numErr )
		return *this;
	::SleepEx(num, TRUE);
	return *this;
}

SubProcess& SubProcess::Flush(){
	if( __numErr )
		return *this;
	if( __ToChildBuf.str().size() ){
		if( !__WriteToCli(__ToChildBuf.str()) ){
			return *this;
		}
		__ToChildBuf.str("");
		__ToChildBuf.clear();
	}
	if( !FlushFileBuffers(__hSevW) ){
		ENOut(__numErr = ::GetLastError());
		return *this;
	}
	return *this;
}

SubProcess& SubProcess::ClearBuffer(){
	__ToChildBuf.str("");
	__ToChildBuf.clear();
	__ClearQue(__FromChildQue);
	__ClearQue(__FromChildErrQue);
	return *this;
}

SubProcess& SubProcess::operator>>(std::string& str){
	str.clear();

	DWORD timeout;
	if( __numAwait ){
		timeout = __numAwait;
		__numAwait = 0;
	} else{
		timeout = __numTimeOut;
	}

	if( __TryReadOperation(timeout) ){
		if( __bfIsErrOut ){
			__bfIsErrOut = 0;
			__MoveString(__FromChildErrQue, str);
			return *this;
		} else{
			__MoveString(__FromChildQue, str);
			return *this;
		}
	} else{
		__numErr = WAIT_TIMEOUT;
	}
	return *this;
}

SubProcess& SubProcess::operator>>(std::ostream& os){
	std::string str;
	operator >> (str);
	os << str;
	return *this;
}

SubProcess& SubProcess::Await(DWORD numAwaitTime)noexcept{
	__numAwait = numAwaitTime;
	return *this;
}

SubProcess& SubProcess::CErr()noexcept{
	__bfIsErrOut = true;
	return *this;
}

bool SubProcess::IsActive(){
	if( !__PI.hProcess )
		return false;
	DWORD dw;
	if( !::GetExitCodeProcess(__PI.hProcess, &dw) ){
		ENOut(__numErr = ::GetLastError());
		return false;
	}
	return dw == STILL_ACTIVE;
}

bool SubProcess::IsReadable(DWORD time){

	DWORD timeout;
	if( __numAwait ){
		timeout = __numAwait;
		__numAwait = 0;
	} else{
		timeout = time;
	}

	__TryReadOperation(timeout);
	if( __bfIsErrOut ){
		__bfIsErrOut = false;
		return __FromChildErrQue.size();
	} else{
		return __FromChildQue.size();
	}
}

bool SubProcess::SetUseStdErr(bool is_use)noexcept{
	if( __numErr )
		return false;
	if( IsActive() ){
		_D("Cannot be set after the SubProcess has been launched.");
		__numErr = STILL_ACTIVE;
		return false;
	}
	__bfIsUseStdErr = is_use;
	return true;
}

inline std::wstring SubProcess::__AtoW(const std::string& str)const{
	if( str.empty() ){
		return std::wstring();
	}
	std::wstring wstr(MAX_PATH, L'\0');
	//ANSIからユニコードへ変換
	int size(0);
	if( !(size = ::MultiByteToWideChar(
		CP_ACP
		, 0
		, str.data()
		, static_cast<int>(str.size())
		, wstr.data()
		, static_cast<int>(wstr.size()))) ){
		ENOut(::GetLastError());
		wstr.resize(0);
		return wstr;
	}
	wstr.resize(size);
	return wstr;
}

bool SubProcess::__ReadFromChild(){
	OVERLAPPED_CUSTOM* pOL = &(*(__mlOL.Lend()) = {});
	pOL->self = this;
	pOL->dir = Dir::COUT;
	if( !::ReadFileEx(
		__hSevR
		, &(pOL->buffer)
		, sizeof(pOL->buffer)
		, (OVERLAPPED*)pOL
		, __pfReadFromChildCompleted) ){
		ENOut(__numErr = ::GetLastError());
		__mlOL.Return(pOL);
		return false;
	}
	__TryReadOperation(CONTINUOUS_TIMEOUT);
	return true;
}

bool SubProcess::__ReadFromChildErr(){
	OVERLAPPED_CUSTOM* pOL = &(*(__mlOL.Lend()) = {});
	pOL->self = this;
	pOL->dir = Dir::CERROR;
	if( !::ReadFileEx(
		__hErrR
		, &(pOL->buffer)
		, sizeof(pOL->buffer)
		, (OVERLAPPED*)pOL
		, __pfReadFromChildCompleted) ){
		ENOut(__numErr = ::GetLastError());
		__mlOL.Return(pOL);
		return false;
	}
	__TryReadOperation(CONTINUOUS_TIMEOUT);
	return true;
}

void SubProcess::__ClearQue(std::queue<OVERLAPPED_CUSTOM*>& que){
	for( size_t sz = que.size(); sz; --sz ){
		__mlOL.Return(que.front());
		que.pop();
	}
}

DWORD SubProcess::__MoveString(std::queue<OVERLAPPED_CUSTOM*>& que, std::string& str){
	str.clear();
	size_t TotalSize(0);
	for( size_t i = que.size(); 0 < i; --i ){
		OVERLAPPED_CUSTOM* p = que.front();
		TotalSize += p->size;
		que.pop();
		que.push(p);
	}
	str.reserve(TotalSize);
	for( size_t i = que.size(); 0 < i; --i ){
		OVERLAPPED_CUSTOM* p = que.front();
		str.append(p->buffer, p->size);
		que.pop();
		__mlOL.Return(p);
	}
	return (DWORD)TotalSize;
}

bool SubProcess::__TryReadOperation(DWORD timer){
	for( ;;){

		switch( ::SleepEx(timer, TRUE) ){
		case WAIT_IO_COMPLETION:
		{
			continue;
		}
		case 0:
		{
			if( __bfIsErrOut ){
				return __FromChildErrQue.size();
			} else{
				return __FromChildQue.size();
			}
		}
		default:
			return false;
		}
	}
}

std::wstring SubProcess::__CreateNamedPipeStringW(){
	GUID guid;
	std::wstring wstr(0x100, L'\0');
	if( CoCreateGuid(&guid) != S_OK ){
		_MES("CoCreateGuid Err.");
		throw std::runtime_error("CoCreateGuid Err.");
	};
	wstr.resize(StringFromGUID2(guid, wstr.data(), (int)wstr.capacity()));
	if( !wstr.size() ){
		_MES("StringFromGUID2 Err.");
		throw std::runtime_error("StringFromGUID2 Err.");
	}
	wstr = std::wstring(L"\\\\.\\pipe\\") + __AtoW(__FILE__) + wstr.c_str() + L"\\";
	return wstr;
}

bool SubProcess::__ClosePipes(){
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

bool SubProcess::__CancelIo(){
	::CancelIoEx(__hCliR, NULL);
	::CancelIoEx(__hCliW, NULL);
	::CancelIoEx(__hSevR, NULL);
	::CancelIoEx(__hErrR, NULL);
	::CancelIoEx(__hSevW, NULL);
	return true;
}

void SubProcess::ResetFlag()noexcept{
	__numErr = 0;
}

DWORD SubProcess::SetTimeOut(DWORD uiTime)noexcept{
	DWORD tmp = __numTimeOut;
	__numTimeOut = uiTime;
	return tmp;
}

bool SubProcess::__WriteToCli(const std::string& str){
	// バッファのサイズをチャンクサイズとする
	const std::size_t chunkSize = sizeof(OVERLAPPED_CUSTOM::buffer);
	std::size_t position = 0;

	for( ; position < str.size();){
		OVERLAPPED_CUSTOM* pOL = &(*(__mlOL.Lend()) = {});
		std::size_t size = std::min<size_t>(chunkSize, str.size() - position);
		std::copy(str.begin() + position, str.begin() + position + size, pOL->buffer);
		pOL->self = this;
		if( !WriteFileEx(
			__hSevW
			, &(pOL->buffer)
			, (DWORD)size
			, (OVERLAPPED*)pOL
			, __pfWriteToChildCompleted) ){
			ENOut(__numErr = GetLastError());
			__mlOL.Return(pOL);
			return FALSE;
		}

		__TryReadOperation(CONTINUOUS_TIMEOUT);

		position += size;
	}
	return true;
}

std::ostream& operator<<(std::ostream& os, SubProcess& sp){
	if( sp.__numErr )
		return os;
	std::string str;
	sp >> str;
	os << str;
	return os;
}