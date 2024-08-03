/**
 * @file OrderedLockMutex.h
 * @brief ���ԕۏ�~���[�e�b�N�X���b�N�N���X�̐錾
 * @author Gold Smith<br>
 * @date 2022-2024<br>
 * @copyright SPDX-License-Identifier: MIT<br>
 * Released under the MIT license<br>
 * https: //opensource.org/licenses/mit-license.php<br>
 * ���̃t�@�C�����̂��ׂẴR�[�h�́A���ɖ��L����Ă��Ȃ�����AMIT���C�Z���X�ɏ]���܂��B
 */
#include <Windows.h>
#include <memory>
#include <iostream>
#include <sstream>
#include "defSTRINGIZE.h"

#pragma comment(lib,  "../Debug_fnc/" STRINGIZE($CONFIGURATION) "/Debug_fnc-" STRINGIZE($CONFIGURATION) ".lib")
#pragma once
// ���b�N��ێ�����N���X
class OrderedLock{
public:
	OrderedLock();
	void doWorkInOrder(int num, int i);
private:
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> hMtx1;
	std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(CloseHandle)*> hMtx2;
};
