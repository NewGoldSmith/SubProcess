/**
 * @file sampleIOCP.cpp
 * @brief sampleIOCP作成クラス実装
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#include "sampleIOCP.h"
using namespace std;
using namespace debug_fnc;
sampleIOCP::sampleIOCP():

	__pfReadFromCliCompleted{ [](
		DWORD errorCode
		, DWORD bytesTransfered
		, OVERLAPPED* overlapped){

		unique_ptr<OVERLAPPED_CUSTOM ,void(*)(OVERLAPPED_CUSTOM*)> pOL = {
			reinterpret_cast<OVERLAPPED_CUSTOM*>(overlapped)
			, [](OVERLAPPED_CUSTOM* p)->void{p->self->__mlOL.Return(p); }};

		if( errorCode != ERROR_SUCCESS ){
			debug_fnc::ENOut(errorCode);
			pOL->self->__numErr = errorCode;
			return;
		}
		pOL->self->__ssFromCli.write(pOL->buffer, bytesTransfered);

		pOL->self->OrCout.Push("Sev:ReadFromCliCompleted.\""
			+ std::string(pOL->buffer, bytesTransfered) + "\"");

		pOL->self->__ReadFromCli();

	} }

	, __pfWriteToCliCompleted{ [](
		DWORD errorCode
		, DWORD bytesTransfered
		, OVERLAPPED* overlapped){

		std::unique_ptr<OVERLAPPED_CUSTOM ,void(*)(OVERLAPPED_CUSTOM*)> pOL = {
			reinterpret_cast<OVERLAPPED_CUSTOM*>(overlapped)
			, [](OVERLAPPED_CUSTOM* p)->void{p->self->__mlOL.Return(p); } };

		if( errorCode != ERROR_SUCCESS ){
			debug_fnc::ENOut(errorCode);
			pOL->self->__numErr = errorCode;
			return;
		}

		pOL->self->OrCout.Push("Sev:WriteToCliCompleted. \""
									+ std::string(pOL->buffer, bytesTransfered)
									+ "\"\n");
	} }

	, __pThreadCli{ [](void* pvoid)->DWORD{
		sampleIOCP* pThis = reinterpret_cast<sampleIOCP*>(pvoid);
		std::string str;
		for( ;;){
			if( !pThis->__ReadCliSide(str) ){
				return 1;
			}
			pThis->OrCout.Push("Cli:Client successfully read.\"" + str + "\"\n");

			::Sleep(CLIENT_WORK_TIME);

			if( !pThis->__WriteCliSide(str) ){
				return 2;
			}
			pThis->OrCout.Push("Cli:Client successfully wrote.\"" + str + "\"\n");
		}
		return 0;
	} }

	, __mlOL(__OLArr, NUM_OVERLAPPED)

#ifdef USING_IOCP
	, __hIOCP{ [](){
	HANDLE h;
	if( !(h = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) ){
		DWORD dw = GetLastError();
		throw std::exception(debug_fnc::ENOut(dw).c_str());};	return h; }()
			, ::CloseHandle }
#endif // USING_IOCP
{
	OrCout.Trigger(true);
	if( !__CreatePipes() )
		throw std::exception("__CreatePipes");
	if (!__StartCliThread())
		throw std::runtime_error("The creation of the subthread failed.");

#ifdef USING_IOCP
	if (!(::CreateIoCompletionPort(__hSevR, __hIOCP.get(), __dir::R, 0)) ){
		throw std::exception(debug_fnc::ENOut(__numErr = ::GetLastError()).c_str());
	}
	if (!(::CreateIoCompletionPort(__hSevW, __hIOCP.get(), __dir::W, 0)) ){
		throw std::exception(debug_fnc::ENOut(__numErr = ::GetLastError()).c_str());
	}
#endif // USING_IOCP
	__ReadFromCli();
}

sampleIOCP::~sampleIOCP() {
	__CancelMultipleIO();
	__ClosePipes();
	__EndCliThread();
}

std::wstring sampleIOCP::__CreateNamedPipeStringW() {
	GUID guid;
	std::wstring wstr(MAX_STRING, L'\0');
	if (::CoCreateGuid(&guid) != S_OK) {
		debug_fnc::dout("CoCreateGuid Err.");
		throw std::runtime_error("CoCreateGuid Err.");
	};
	wstr.resize(::StringFromGUID2(guid, wstr.data(), (int)wstr.capacity()));
	if (!wstr.size()) {
		debug_fnc::dout("StringFromGUID2 Err.");
		throw std::runtime_error("StringFromGUID2 Err.");
	}
	return std::wstring(L"\\\\.\\pipe\\") + L"sampleIOCP" + wstr.c_str() + L"\\";
}

bool sampleIOCP::__CreatePipes() {
	{
		std::wstring wstr = __CreateNamedPipeStringW();
		if ((__hSevW = ::CreateNamedPipeW(
			wstr.c_str()
			, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED
			, PIPE_TYPE_BYTE | PIPE_WAIT | PIPE_ACCEPT_REMOTE_CLIENTS
			, PIPE_UNLIMITED_INSTANCES
			, BUFER_SIZE_PIPE
			, BUFER_SIZE_PIPE
			, 0
			, NULL)) == INVALID_HANDLE_VALUE) {
			debug_fnc::ENOut(__numErr = ::GetLastError());
			return FALSE;
		}

		if ((__hCliR = ::CreateFileW(
			wstr.c_str()
			, PIPE_ACCESS_DUPLEX
			, FILE_SHARE_WRITE | FILE_SHARE_READ
			, NULL
			, OPEN_EXISTING
			, FILE_ATTRIBUTE_NORMAL
			, NULL
		)) == INVALID_HANDLE_VALUE) {
			debug_fnc::ENOut(__numErr = ::GetLastError());
			return FALSE;
		};
	}
	{
		std::wstring wstr = __CreateNamedPipeStringW();
		if ((__hSevR = ::CreateNamedPipeW(
			wstr.c_str()
			, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED
			, PIPE_TYPE_BYTE | PIPE_WAIT | PIPE_ACCEPT_REMOTE_CLIENTS
			, PIPE_UNLIMITED_INSTANCES
			, BUFER_SIZE_PIPE
			, BUFER_SIZE_PIPE
			, 0
			, NULL)) == INVALID_HANDLE_VALUE) {
			debug_fnc::ENOut(__numErr = ::GetLastError());
			return FALSE;
		}

		if ((__hCliW = ::CreateFileW(
			wstr.c_str()
			, PIPE_ACCESS_DUPLEX
			, FILE_SHARE_WRITE | FILE_SHARE_READ
			, NULL
			, OPEN_EXISTING
			, FILE_ATTRIBUTE_NORMAL
			, NULL
		)) == INVALID_HANDLE_VALUE) {
			debug_fnc::ENOut(__numErr = ::GetLastError());
			return FALSE;
		};
	}
	return true;
}

bool sampleIOCP::__CancelMultipleIO(){
	::CancelIoEx(__hSevR, NULL);
	::CancelIoEx(__hSevW, NULL);

#ifdef USING_IOCP
	if( !::CancelSynchronousIo(__hThreadCli) ){
		debug_fnc::ENOut(__numErr = ::GetLastError());
	}

	for( ;;){
		__numErr = __WaitForIOCompletion(CONTINUOUS_TIMEOUT);

		switch( __numErr ){
		case 0:
		{
			continue;
		}
		case WAIT_TIMEOUT:
		{
			__numErr = 0;
			goto end_of_loop;
		}
		default:
			goto end_of_loop;
		}
	}
end_of_loop:

#else // !USING_IOCP
	for( ;; ){
		DWORD dw;
		if( (dw = ::SleepEx(CONTINUOUS_TIMEOUT, TRUE)) == WAIT_IO_COMPLETION ){
			continue;
		}
		debug_fnc::ENOut(dw);
		break;
	}
	::CancelIoEx(__hCliR, NULL);
	::CancelIoEx(__hCliW, NULL);
#endif // USING_IOCP
	return true;
}

bool sampleIOCP::__WriteToCli(const std::string& str) {
	// この関数のEnterとLeaveを表示
	std::unique_ptr<OrderedCOut, void(*)(OrderedCOut* p)> pOrder = {
	[this](){this->OrCout.Push("Sev:Enter __WriteToCli"); return &(this->OrCout); }()
	,[](OrderedCOut* p)->void{p->Push("Sev:Leave __WriteToCli"); } };

	// バッファのサイズをチャンクサイズとする
	const std::size_t chunkSize = sizeof(OVERLAPPED_CUSTOM::buffer);
	std::size_t position = 0;

	for( ; position < str.size();){
		OVERLAPPED_CUSTOM* pOL = &(*(__mlOL.Lend()) = {});
		std::size_t size = std::min<size_t>(chunkSize, str.size() - position);
		std::copy(str.begin() + position, str.begin() + position + size, pOL->buffer);
		pOL->self = this;
#ifdef USING_IOCP
		if (!WriteFile(
			__hSevW
			, &(pOL->buffer)
			, (DWORD)size
			, NULL
			, (OVERLAPPED*)pOL)) {
			switch( __numErr = ::GetLastError() ){
			case ERROR_IO_PENDING:
			{
				__numErr = 0;
				break;
			}
			default:
				debug_fnc::ENOut(__numErr);
				return false;
			}
		}
#else // !USING_IOCP
		if (!WriteFileEx(
			__hSevW
			, &(pOL->buffer)
			, (DWORD)size
			, (OVERLAPPED*)pOL
			, __pfWriteToCliCompleted)) {
			ENOut(__numErr = GetLastError());
			return FALSE;
		}
#endif // USING_IOCP
		OrCout.Push("Sev:Server successfully wrote. \"" + string(pOL->buffer, size) + "\"\n");

		bool at_least_one_loop(0);
		for (;;) {
#ifdef USING_IOCP
			__numErr = __WaitForIOCompletion(CONTINUOUS_TIMEOUT);
			switch( __numErr ){
			case 0: {
				continue;
			}
			case WAIT_TIMEOUT: {
				__numErr = 0;
				goto end_of_loop;
			}

			default:
				debug_fnc::ENOut(__numErr);
				return false;
			}
#else // USING_IOCP
			__numErr = ::SleepEx(CONTINUOUS_TIMEOUT, TRUE);
			switch (__numErr) {
			case WAIT_IO_COMPLETION:
			{
				__numErr = 0;
				at_least_one_loop = true;
				continue;
			}
			case 0:
			{
				goto end_of_loop;
			}
			default:
				debug_fnc::ENOut(__numErr);
				goto end_of_loop;
			}
#endif // !USING_IOCP
		}
	end_of_loop:

		position += size;
	}
	return true;
}

#ifdef USING_IOCP
DWORD sampleIOCP::__WaitForIOCompletion(DWORD time){
	DWORD numberOfBytes;
	ULONG_PTR completionKey;
	LPOVERLAPPED lpOverlapped;
	DWORD dw(0);
	if( !::GetQueuedCompletionStatus(__hIOCP.get(), &numberOfBytes, &completionKey, &lpOverlapped, time) ){
		dw= GetLastError();
	}

	if( lpOverlapped ){
		if( completionKey == __dir::R )
			(__pfReadFromCliCompleted)(dw, numberOfBytes, lpOverlapped);
		else
			(__pfWriteToCliCompleted)(dw, numberOfBytes, lpOverlapped);
	}
	return dw;
}
#endif // USING_IOCP


bool sampleIOCP::__ReadFromCli() {
	// この関数のEnterとLeaveを表示
	std::unique_ptr<OrderedCOut, void(*)(OrderedCOut* p)> pOrder = {
		[this]() {this->OrCout.Push("Sev:Enter __ReadFromCli"); return &(this->OrCout); }()
		,[](OrderedCOut* p)->void {p->Push("Sev:Leave __ReadFromCli"); } };

	if (__numErr)
		return false;
	OVERLAPPED_CUSTOM* pOL = &(*(__mlOL.Lend()) = {});
	pOL->self = this;
#ifdef USING_IOCP
	if (!::ReadFile(
		__hSevR
		, &(pOL->buffer)
		, sizeof(pOL->buffer)
		, NULL
		, (OVERLAPPED*)pOL)) {
		switch (__numErr = ::GetLastError()) {
		case ERROR_IO_PENDING:
		{
			__numErr = 0;
			break;
		}
		default:
			debug_fnc::ENOut(__numErr);
			return false;
		}
	}
#else // !USING_IOCP
	if (!::ReadFileEx(
		__hSevR
		, &(pOL->buffer)
		, sizeof(pOL->buffer)
		, (OVERLAPPED*)pOL
		, __pfReadFromCliCompleted)) {
		debug_fnc::ENOut(__numErr = ::GetLastError());
		return false;
	}
#endif // USING_IOCP

	for (;;) {

#ifdef USING_IOCP
		__numErr = __WaitForIOCompletion(CONTINUOUS_TIMEOUT);
		switch ( __numErr ) {
		case WAIT_TIMEOUT:
		{
			__numErr = 0;
			goto end_of_loop;
		}
		default:
			debug_fnc::ENOut(__numErr);
			return false;
		}

#else // !USING_IOCP
		__numErr = ::SleepEx(CONTINUOUS_TIMEOUT, TRUE);
		switch (__numErr) {
		case WAIT_IO_COMPLETION:
		{
			OrCout.Push("Sev:successfully operator>>. \"WAIT_IO_COMPLETION\"\n");
			__numErr = 0;
			continue;
		}
		case 0:
		{
			__numErr = 0;
			goto end_of_loop;
		}
		default:
			debug_fnc::ENOut(__numErr);
			goto end_of_loop;
		}
#endif // USING_IOCP

	}

end_of_loop:
	return true;
}

bool sampleIOCP::__StartCliThread()
{
	SECURITY_ATTRIBUTES sa{};
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor;

	if (!(__hThreadCli =
		::CreateThread(
			NULL
			, 0
			, __pThreadCli
			, this
			, 0
			, NULL))) {
		debug_fnc::ENOut(__numErr = ::GetLastError());
		return false;
	}
	return true;
}

bool sampleIOCP::__EndCliThread()
{
	::WaitForSingleObject(__hThreadCli, INFINITE);
	::CloseHandle(__hThreadCli);
	__hThreadCli = NULL;
	return true;
}

bool sampleIOCP::__WriteCliSide(const std::string &str) {
	if (!::WriteFile(
		__hCliW
		, str.data()
		, (DWORD)str.size()
		, NULL
		, NULL)) {
		debug_fnc::ENOut(__numErr = ::GetLastError());
		return FALSE;
	}
	return TRUE;
}

bool sampleIOCP::__ReadCliSide(std::string &str) {
	DWORD dwRead;
	str.resize(BUFFER_SIZE_CL_SIDE, '\0');
	if (!::ReadFile(__hCliR
					  , str.data()
					  , (DWORD)str.size()
					  , &dwRead
					  , NULL)) {
		debug_fnc::ENOut(__numErr = ::GetLastError());
		return FALSE;
	}
	str.resize(dwRead);
	return TRUE;
}

bool sampleIOCP::__ClosePipes() {
	::CloseHandle(__hCliR);
	::CloseHandle(__hCliW);
	::CloseHandle(__hSevR);
	::CloseHandle(__hSevW);
	return true;
}

void sampleIOCP::ResetFlag() {
	__numErr = 0;
}
DWORD sampleIOCP::SetTimeOut(DWORD Time) {
	unsigned tmp = __TimeOut;
	__TimeOut = Time;
	return tmp;
}

sampleIOCP &sampleIOCP::operator<<(const std::string &str) {
	if (__numErr)
		return *this;
	__WriteToCli(str);
	return *this;
}

sampleIOCP &sampleIOCP::operator>>(string &str) {
	// この関数のEnterとLeaveを表示
	std::unique_ptr<OrderedCOut, void(*)(OrderedCOut* p)> pOrder = {
		[this]() {this->OrCout.Push("Sev:Enter operator>>"); return &(this->OrCout); }()
		,[](OrderedCOut* p)->void {p->Push("Sev:Leave operator>>"); } };

	if (__numErr)
		return *this;

	str.clear();

	DWORD timeout(0);
	if (__Await) {
		timeout = __Await;
		__Await = 0;
	} else {
		timeout = __TimeOut;
	}

	bool at_least_one_loop(0);
	for (;;) {

#ifdef USING_IOCP
		__numErr = __WaitForIOCompletion(timeout);
		switch (__numErr) {
		case 0:
		{
			str += __ssFromCli.str();
			__ssFromCli.str("");
			__ssFromCli.clear();
			at_least_one_loop = true;
			continue;
		}
		case WAIT_TIMEOUT:
		{
			if( at_least_one_loop || __ssFromCli.str().size() )
				__numErr = 0;
			str += __ssFromCli.str();
			__ssFromCli.str("");
			__ssFromCli.clear();
			goto end_of_loop;
		}
		default:
			debug_fnc::ENOut(__numErr);
			goto end_of_loop;
		}
#else // USING_IOCP
		__numErr = ::SleepEx(timeout, TRUE);
		switch (__numErr) {
		case WAIT_IO_COMPLETION:
		{
			__numErr = 0;
			str += __ssFromCli.str();
			__ssFromCli.str("");
			__ssFromCli.clear();
			at_least_one_loop = true;
			continue;
		}
		case 0:
		{
			if( at_least_one_loop )
				__numErr = 0;
			if( __ssFromCli.str().size() ){
				str += __ssFromCli.str();
			}
			goto end_of_loop;
		}
		default:
			debug_fnc::ENOut(__numErr);
			goto end_of_loop;
		}
#endif // !USING_IOCP
	}
end_of_loop:

	return *this;
}

sampleIOCP& sampleIOCP::Await(DWORD mtime) {
	__Await = mtime;
	return *this;
}



