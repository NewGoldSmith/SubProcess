/**
 * @file sampleOverLapped.h
 * @brief sampleOverLappedçÏê¨ÉNÉâÉXêÈåæ
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#pragma once
#define NOMINMAX
#include <Windows.h>
#include <string>
#include <iostream>
#include <queue>
#include "../Debug_fnc/debug_fnc.h"
#include "./MemoryLoan.h"
#pragma comment(lib,  "../Debug_fnc/" _CRT_STRINGIZE($CONFIGURATION) "/Debug_fnc-" _CRT_STRINGIZE($CONFIGURATION) ".lib")

using namespace std;

class  sampleIOCP {
	static constexpr int BUFFER_SIZE = 0x100;
	static constexpr int NUM_OVERLAPPED = 0x20;
	static constexpr int DEFAULT_TIMEOUT = 1000;
public:
	sampleIOCP() ;
	~sampleIOCP() { ClosePipes(); };
	wstring CreateNamedPipeString();
	bool CreatePipes();
	bool WriteCli(const string &str) const;
	bool ReadCli(string &str);
	bool ClosePipes();
	bool CancelSev();
	void ResetFlag();
	unsigned SetTimeOut(unsigned uiTime);
	bool IsTimeOut() { return fIsTimeOut; };
	sampleIOCP &operator<<(const string &str);
	sampleIOCP &operator>>(string &str);
	explicit operator bool() const { return fIsSucceed; }
private:
	bool WriteToCli(const string &str);
	bool ReadFromCli();
	struct OVERLAPPED_CUSTOM {
		OVERLAPPED ol{};
		HANDLE h{};
		//DWORD DataSize{};
		sampleIOCP *pThis{};
		unsigned char buffer[BUFFER_SIZE]{};
	};

	HANDLE hSevR{}, hSevW{}, hCliR{}, hCliW{};
	bool fIsSucceed{ true };
	bool fIsTimeOut{ false };
	int TimeOut{ DEFAULT_TIMEOUT };
	PROCESS_INFORMATION PI{};
	LPOVERLAPPED_COMPLETION_ROUTINE const pfReadFromChildCompleted;
	LPOVERLAPPED_COMPLETION_ROUTINE const pfWriteToChildCompleted;
	MemoryLoan< OVERLAPPED_CUSTOM> mlOL;
	queue<string> strOutArr{};
	OVERLAPPED_CUSTOM OLArr[NUM_OVERLAPPED];
};
