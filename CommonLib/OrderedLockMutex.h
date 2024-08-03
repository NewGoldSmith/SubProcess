/**
 * @file OrderedLockMutex.h
 * @brief 順番保障ミューテックスロッククラスの宣言
 * @author Gold Smith<br>
 * @date 2022-2024<br>
 * @copyright SPDX-License-Identifier: MIT<br>
 * Released under the MIT license<br>
 * https: //opensource.org/licenses/mit-license.php<br>
 * このファイル内のすべてのコードは、特に明記されていない限り、MITライセンスに従います。
 */
#include <Windows.h>
#include <memory>
#include <iostream>
#include <sstream>
#include "defSTRINGIZE.h"

#pragma comment(lib,  "../Debug_fnc/" STRINGIZE($CONFIGURATION) "/Debug_fnc-" STRINGIZE($CONFIGURATION) ".lib")
#pragma once
// ロックを保持するクラス
class OrderedLock{
public:
	OrderedLock();
	void doWorkInOrder(int num, int i);
private:
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> hMtx1;
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> hMtx2;
};
