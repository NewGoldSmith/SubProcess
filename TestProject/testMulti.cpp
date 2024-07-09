/**
 * @file testMulti.cpp
 * @brief IOCPテスト実装
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#include "testIOCP.h"
using namespace std;
int main() {
	{
		HANDLE hSem;
		if (!(hSem = CreateSemaphoreA(NULL, 0, 5, NULL))) {
			DWORD dw = GetLastError();
			debug_fnc::ENOut(dw);
			return 1;
		}
		LONG release;
		ReleaseSemaphore(hSem,0,&release);
		HANDLE hs[5];
		for (int i = 0; i < 5; ++i) {
			if (!(DuplicateHandle(
				GetCurrentProcess(),  // ソースハンドルのプロセスハンドル
				hSem,  // ソースハンドル
				GetCurrentProcess(),  // ターゲットハンドルのプロセスハンドル
				&(hs[i]),  // ターゲットハンドル
				0,  // 必要なアクセス権
				FALSE,  // ハンドルの継承オプション
				DUPLICATE_SAME_ACCESS  // オプションフラグ
			))) {
				debug_fnc::ENOut(GetLastError());
				return 1;
			}
		}
		DWORD dw;
		if ((dw = WaitForMultipleObjects(1, hs, true, 1000)) == WAIT_FAILED) {
			DWORD dw2 = GetLastError();
			debug_fnc::ENOut(dw2);
		}
		switch (dw) {
		case WAIT_OBJECT_0:
			break;
		case WAIT_TIMEOUT:
			return 1;
		default:
			debug_fnc::ENOut(dw);
			return 1;
		}
	}
	_CrtDumpMemoryLeaks();
	return 0;
}
