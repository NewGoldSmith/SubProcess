/**
 * @file OrderedLockMutex.cpp
 * @brief ���ԕۏ�~���[�e�b�N�X���b�N�N���X�̎���
 * @author Gold Smith<br>
 * @date 2022-2024<br>
 * @copyright SPDX-License-Identifier: MIT<br>
 * Released under the MIT license<br>
 * https: //opensource.org/licenses/mit-license.php<br>
 * ���̃t�@�C�����̂��ׂẴR�[�h�́A���ɖ��L����Ă��Ȃ�����AMIT���C�Z���X�ɏ]���܂��B
 */
#include "OrderedLockMutex.h"

OrderedLock::OrderedLock():
	hMtx1{ [](){
	HANDLE h;
	if( !(h = CreateMutexA(NULL, FALSE, NULL)) ){
		throw std::exception("CreateMutex");
		} return h; }(), CloseHandle }

	,hMtx2{ [](){
	HANDLE h;
	if( !(h = CreateMutexA(NULL, FALSE, NULL)) ){
		throw std::exception("CreateMutex");
		} return h; }(), CloseHandle }

{}

void OrderedLock::doWorkInOrder(int num, int i){
	WaitForSingleObject(hMtx1.get(), INFINITE);
	WaitForSingleObject(hMtx2.get(), INFINITE);


	// �����ō�Ƃ��s��
	std::stringstream ss;
	ss << "thread" << std::to_string(num) << " " << std::to_string(i) << std::endl;
	std::cout << ss.str();

	ReleaseMutex(hMtx1.get());
	ReleaseMutex(hMtx2.get());
}
