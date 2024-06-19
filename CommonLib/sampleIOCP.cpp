/**
 * @file sampleOverLapped.h
 * @brief sampleOverLappedçÏê¨ÉNÉâÉXé¿ëï
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#include "sampleIOCP.h"
using namespace std;
using namespace debug_fnc;
sampleIOCP::sampleIOCP():

	pfReadFromChildCompleted{ [](
		DWORD errorCode
		, DWORD bytesTransfered
		, OVERLAPPED *overlapped) {

		unique_ptr<OVERLAPPED_CUSTOM ,void(*)(OVERLAPPED_CUSTOM *)> pOL = {
			[&overlapped]() {OVERLAPPED_CUSTOM *p =
			reinterpret_cast<OVERLAPPED_CUSTOM *>(overlapped);	return p; }()
			, [](OVERLAPPED_CUSTOM *p)->void {p->pThis->mlOL.Return(p); } };

		//pOL->DataSize = bytesTransfered;

		if (errorCode != ERROR_SUCCESS) {
			ErrOut_(errorCode, __FILE__, __LINE__, __FUNCTION__, "operation failed with error.");
			pOL->pThis->fIsSucceed = false;
			return;
		}

		string str;
		str.assign(reinterpret_cast<char *>(&(pOL->buffer)), bytesTransfered);
		pOL->pThis->strOutArr.push(str);
	} }

	, pfWriteToChildCompleted{ [](
		DWORD errorCode
		, DWORD bytesTransfered
		, OVERLAPPED *overlapped) {

		unique_ptr<OVERLAPPED_CUSTOM ,void(*)(OVERLAPPED_CUSTOM *)> pOL = {
			[&overlapped]() {OVERLAPPED_CUSTOM *p =
			reinterpret_cast<OVERLAPPED_CUSTOM *>(overlapped);	return p; }()
			, [](OVERLAPPED_CUSTOM *p)->void {p->pThis->mlOL.Return(p); } };

		//pOL->DataSize = bytesTransfered;
		if (errorCode != ERROR_SUCCESS) {
			ENOut(errorCode);
			pOL->pThis->fIsSucceed = false;
			return;
		}
	} }

	, mlOL(OLArr, NUM_OVERLAPPED)
{
	CreatePipes();
};

wstring sampleIOCP::CreateNamedPipeString() {
	GUID guid;
	wstring wstr(0x100, L'\0');
	if (CoCreateGuid(&guid) != S_OK) {
		_MES("CoCreateGuid Err.");
		throw runtime_error("CoCreateGuid Err.");
	};
	wstr.resize(StringFromGUID2(guid, wstr.data(), (int)wstr.capacity()));
	if (!wstr.size()) {
		_MES("StringFromGUID2 Err.");
		throw runtime_error("StringFromGUID2 Err.");
	}
	wstr = L"\\\\.\\pipe\\"  L"sampleOverLapped" + wstr + L"\\";
	return wstr;
}

bool sampleIOCP::CreatePipes() {
	SECURITY_ATTRIBUTES saAttr = {};
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = FALSE;
	saAttr.lpSecurityDescriptor = NULL;

	{
		saAttr.bInheritHandle = FALSE;
		wstring wstr = CreateNamedPipeString();
		if ((hSevW = CreateNamedPipeW(
			wstr.c_str()
			, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED
			, PIPE_TYPE_MESSAGE | PIPE_WAIT | PIPE_ACCEPT_REMOTE_CLIENTS
			, PIPE_UNLIMITED_INSTANCES
			, BUFFER_SIZE
			, BUFFER_SIZE
			, 0
			, NULL)) == INVALID_HANDLE_VALUE) {
			EOut;
			return FALSE;
		}

		saAttr.bInheritHandle = TRUE;
		if ((hCliR = CreateFileW(
			wstr.c_str(),   // pipe name 
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,  // read and write access 
			FILE_SHARE_WRITE | FILE_SHARE_READ,              // no sharing 
			&saAttr,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			FILE_ATTRIBUTE_NORMAL,              // default attributes 
			NULL// no template file 
		)) == INVALID_HANDLE_VALUE) {
			EOut;
			return FALSE;
		};
	}
	{
		saAttr.bInheritHandle = FALSE;
		wstring wstr = CreateNamedPipeString();
		if ((hSevR = CreateNamedPipeW(
			wstr.c_str()
			, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED
			, PIPE_TYPE_MESSAGE | PIPE_WAIT | PIPE_ACCEPT_REMOTE_CLIENTS
			, PIPE_UNLIMITED_INSTANCES
			, BUFFER_SIZE
			, BUFFER_SIZE
			, 0
			, NULL)) == INVALID_HANDLE_VALUE) {
			EOut;
			return FALSE;
		}

		saAttr.bInheritHandle = TRUE;
		if ((hCliW = CreateFileW(
			wstr.c_str(),   // pipe name 
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,  // read and write access 
			FILE_SHARE_WRITE | FILE_SHARE_READ,              // no sharing 
			&saAttr,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			FILE_ATTRIBUTE_NORMAL,              // default attributes 
			NULL// no template file 
		)) == INVALID_HANDLE_VALUE) {
			EOut;
			return FALSE;
		};
	}
	return true;
}

bool sampleIOCP::WriteToCli(const string &str) {
	OVERLAPPED_CUSTOM *pOL = &(*(mlOL.Lend()) = {});
	//pOL->DataSize = (DWORD)str.size();
	copy(str.begin(), str.end(), pOL->buffer);
	pOL->h = hSevW;
	pOL->pThis = this;
	DWORD dw;
	if (!WriteFileEx(
		pOL->h
		, &(pOL->buffer)
		, (DWORD)str.size()
		, (OVERLAPPED *)pOL
		, pfWriteToChildCompleted)) {
		dw = GetLastError();
		if (dw != WAIT_IO_COMPLETION) {
			ENOut(dw);
			return FALSE;
		}
	}
	return true;
}

bool sampleIOCP::ReadFromCli() {
	OVERLAPPED_CUSTOM *pOL = &(*(mlOL.Lend()) = {});
	pOL->h = hSevR;
	pOL->pThis = this;
	//pOL->DataSize = 1024;
	DWORD dw;
	if (!ReadFileEx(
		pOL->h
		, &(pOL->buffer)
		, BUFFER_SIZE
		, (OVERLAPPED *)pOL
		, pfReadFromChildCompleted)) {
		dw = GetLastError();
		if (dw != WAIT_IO_COMPLETION) {
			ENOut(dw);
			return FALSE;
		}
	}
	return true;
}

bool sampleIOCP::WriteCli(const string &str) const {
	DWORD cbWrites;
	if (!WriteFile(
		hCliW
		, str.data()
		, (DWORD)str.size()
		, &cbWrites
		, NULL)) {
		EOut;
		return FALSE;
	}
	return TRUE;
}

bool sampleIOCP::ReadCli(string &str) {
	DWORD dwRead;
	str.resize(1024, '\0');
	if (!ReadFile(hCliR
					  , str.data()
					  , (DWORD)str.size()
					  , &dwRead
					  , NULL)) {
		EOut;
		return FALSE;
	}
	str.resize(dwRead);
	return TRUE;
}

bool sampleIOCP::ClosePipes() {
	CancelIoEx(hSevR, NULL);
	CancelIoEx(hSevW, NULL);
	SleepEx(0, TRUE);
	CloseHandle(hSevR);
	CloseHandle(hSevW);
	CloseHandle(hCliR);
	CloseHandle(hCliW);
	return true;
}

bool sampleIOCP::CancelSev() {
	if (!CancelIoEx(hSevR, NULL)) {
		EOut;
	}
	if (!CancelIoEx(hSevW, NULL)) {
		EOut;
	}
	SleepEx(0, TRUE);
	return true;
}

void sampleIOCP::ResetFlag() {
	bool tmp = fIsSucceed;
	fIsSucceed = true;
	fIsTimeOut = false;
}

inline unsigned sampleIOCP::SetTimeOut(unsigned uiTime) {
	unsigned tmp = TimeOut;
	TimeOut = uiTime;
	return tmp;
}

sampleIOCP &sampleIOCP::operator<<(const string &str) {
	if (!fIsSucceed)
		return *this;
	if (!WriteToCli(str)) {
		fIsSucceed = false;
		return *this;
	};
	return *this;
}

sampleIOCP &sampleIOCP::operator>>(string &str) {
	if (!fIsSucceed)
		return *this;

	if (strOutArr.empty()) {
		if (!ReadFromCli()) {
			fIsSucceed = false;
			return *this;
		}
		SleepEx(TimeOut, TRUE);
		if (strOutArr.empty()) {
			fIsSucceed = false;
			fIsTimeOut = true;
			return *this;
		}
	}

	str = strOutArr.front();
	strOutArr.pop();
	return *this;
}

//int main() {
//	{
//		sampleIOCP s;
//		if (!(s << "ToCli1"))
//			return 1;
//		string str;
//		s.ReadCli(str);
//		cout << str << endl;
//		s.WriteCli("Reply from child");
//		string str2;
//		if (!(s >> str2))
//			return 1;
//		cout << str2 << endl;
//	}
//	_CrtDumpMemoryLeaks();
//	return 0;
//}





