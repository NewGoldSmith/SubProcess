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

	pfReadFromCliCompleted{ [](
		DWORD errorCode
		, DWORD bytesTransfered
		, OVERLAPPED *overlapped) {

		unique_ptr<OVERLAPPED_CUSTOM ,void(*)(OVERLAPPED_CUSTOM*)> pOL = {
			reinterpret_cast<OVERLAPPED_CUSTOM*>(overlapped)
			, [](OVERLAPPED_CUSTOM* p)->void {p->self->__mlOL.Return(p); }};

		if (errorCode != ERROR_SUCCESS) {
			debug_fnc::ENOut(errorCode);
			pOL->self->__numErr = errorCode;
			return;
		}
		pOL->self->OrCout.Push("Sev:ReadFromCliCompleted.\""
			+std::string(pOL->buffer, bytesTransfered)+ "\"");
		
		pOL->self->__ssFromCli.write(pOL->buffer, bytesTransfered);
		pOL->self->__ReadFromCli();
	} }

	, pfWriteToCliCompleted{ [](
		DWORD errorCode
		, DWORD bytesTransfered
		, OVERLAPPED *overlapped) {


		std::unique_ptr<OVERLAPPED_CUSTOM ,void(*)(OVERLAPPED_CUSTOM*)> pOL = {
			reinterpret_cast<OVERLAPPED_CUSTOM*>(overlapped)
			, [](OVERLAPPED_CUSTOM* p)->void {p->self->__mlOL.Return(p); } };

		if (errorCode != ERROR_SUCCESS) {
			debug_fnc::ENOut(errorCode);
			pOL->self->__numErr = errorCode;
			return;
		}

		pOL->self->OrCout.Push("Sev:WriteToCliCompleted. \""
									+ std::string(pOL->buffer, bytesTransfered)
									+ "\"\n");
	} }

	, pThreadCli{ [](void* pvoid)->DWORD {
		sampleIOCP* pThis = reinterpret_cast<sampleIOCP*>(pvoid);
		std::string str;
		for (;;) {
			if (!pThis->__ReadCliSide(str)) {
				return 1;
			}
			pThis->OrCout.Push("Cli:Client successfully read.\"" + str + "\"\n");

			::Sleep(CLIENT_WORK_TIME);

			if (!pThis->__WriteCliSide(str)) {
				return 2;
			}
			pThis->OrCout.Push("Cli:Client successfully wrote.\"" + str + "\"\n");
		}
		return 0;
	} }

	, __mlOL(OLArr, NUM_OVERLAPPED)
{
	OrCout.Trigger(true);
	CreatePipes();
	if (!__StartCliThread())
		throw std::runtime_error("The creation of the subthread failed.");
	__ReadFromCli();
}
sampleIOCP::~sampleIOCP() {
	ClosePipes();
	__EndCliThread();
}

wstring sampleIOCP::CreateNamedPipeString() {
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
	wstr = std::wstring(L"\\\\.\\pipe\\") + L"sampleIOCP" + wstr.c_str() + L"\\";
	return wstr;
}

bool sampleIOCP::CreatePipes() {
	{
		std::wstring wstr = CreateNamedPipeString();
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
		std::wstring wstr = CreateNamedPipeString();
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

bool sampleIOCP::__WriteToCli(const std::string& str) {
	const std::size_t chunkSize = sizeof(OVERLAPPED_CUSTOM::buffer);  // バッファのサイズをチャンクサイズとする
	std::size_t position = 0;

	for (; position < str.size();) {
		OVERLAPPED_CUSTOM* pOL = &(*(__mlOL.Lend()) = {});
		std::size_t size = std::min<size_t>(chunkSize, str.size() - position);
		std::copy(str.begin() + position, str.begin() + position + size, pOL->buffer);
		pOL->self = this;
		if (!WriteFileEx(
			__hSevW
			, &(pOL->buffer)
			, (DWORD)size
			, (OVERLAPPED*)pOL
			, pfWriteToCliCompleted)) {
			ENOut(__numErr = GetLastError());
			return FALSE;
		}
		OrCout.Push("Sev:called __WriteToCli \"" + string(pOL->buffer, size) + "\"\n");
		position += size;
		::SleepEx(CONTINUOUS_TIMEOUT, TRUE);
	}
	return true;
}



bool sampleIOCP::__ReadFromCli() {
	if (__numErr)
		return false;
	OVERLAPPED_CUSTOM *pOL = &(*(__mlOL.Lend()) = {});
	pOL->self = this;
	if (!::ReadFileEx(
		__hSevR
		, &(pOL->buffer)
		, sizeof(pOL->buffer)
		, (OVERLAPPED *)pOL
		, pfReadFromCliCompleted)) {
		debug_fnc::ENOut(__numErr = ::GetLastError());
		return false;
	}
	OrCout.Push("Sev:called __ReadFromCli");
	return true;
}

bool sampleIOCP::__StartCliThread()
{
	if (!(__hThreadCli =
		::CreateThread(
			NULL
			, 0
			, pThreadCli
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
	DWORD cbWrites(0);
	if (!::WriteFile(
		__hCliW
		, str.data()
		, (DWORD)str.size()
		, &cbWrites
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

bool sampleIOCP::ClosePipes() {
	::CancelIoEx(__hSevR, NULL);
	::SleepEx(CONTINUOUS_TIMEOUT, TRUE);
	::CancelIoEx(__hSevW, NULL);
	::SleepEx(CONTINUOUS_TIMEOUT, TRUE);
	::CancelIoEx(__hCliR, NULL);
	::SleepEx(CONTINUOUS_TIMEOUT, TRUE);
	::CancelIoEx(__hCliW, NULL);
	::SleepEx(CONTINUOUS_TIMEOUT, TRUE);
	::CloseHandle(__hCliR);
	::CloseHandle(__hCliW);
	::CloseHandle(__hSevR);
	::CloseHandle(__hSevW);
	return true;
}

void sampleIOCP::ResetFlag() {
	__numErr = 0;
}

inline unsigned sampleIOCP::SetTimeOut(unsigned uiTime) {
	unsigned tmp = __TimeOut;
	__TimeOut = uiTime;
	return tmp;
}

sampleIOCP &sampleIOCP::operator<<(const std::string &str) {
	if (__numErr)
		return *this;
	__WriteToCli(str);
	return *this;
}

sampleIOCP &sampleIOCP::operator>>(string &str) {
	if (__numErr)
		return *this;

	OrCout.Push("Sev:Call SleepEx\n");
	__numErr = ::SleepEx(__TimeOut, TRUE);
	switch (__numErr) {
	case WAIT_IO_COMPLETION:
	{
		OrCout.Push("Sev:called SleepEx \"WAIT_IO_COMPLETION\"\n");
		__numErr = 0;
		str = __ssFromCli.str();
		__ssFromCli.str("");
		__ssFromCli.clear();
		break;
	}
	case 0:
	{
		std::cout << "Sev:\"ERROR_TIMEOUT\"" << std::endl;
		break;
	}
	default:
		break;
	}
	return *this;
}



