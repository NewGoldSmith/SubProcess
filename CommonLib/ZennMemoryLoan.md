# ���̋L���̑Ώۓǎ�
�@Windows API���g���ہA�u�����A�������[�v�[�����ق�������B�v�Ǝv���A�������[�v�[�������A�������������L���ł��B�\�[�X�R�[�h�̏C������ł�Windows��ˑ��ɂ��鎖���\�ł��B
## �Œ�T�C�Y�̃������[�u���b�N�����x���g���܂킷�A�������[�v�[���B
�@Windows�̔񓯊�IO�́AOVERLAPPED�\����(���ۂ́A�قƂ�ǂ�����܂����\���́j���g���܂��B����́A�m���v���G���e�B�u�V���O���X���b�h�ł��T�[�o�[�p�r�Ɏg����g���b�L�[�Ȏd�g�݂Ȃ�ł����A���͂����f�U�C�����Â��ł���ˁB
�@�Ⴆ�΁A�T�[�o�[�p�r�ŃN���C�A���g�̑҂��󂯂Ɏg���ꍇ�A�P�ڑ����ɂ���OVERLAPPED�\���̂��K�v�ŁA���̍\���̂ւ̃A�h���X��Windows�V�X�e���Ɉ����Ƃ��ēn���܂��B���̍\���̂�ID���o�̎d�g�݂����˂Ă��āA���[�U�[���C�ӂ�ID���o�̎d�g�݂��d����ł����΁A�ǂ̃N���C�A���g���烁�b�Z�[�W���͂��������m�鎖���o���܂��B�T�[�o�[�p�r�ł́A��ʂɂ���OVERLAPPED�\���̂��K�v�ɂȂ�܂��B�܂��A���̍\���̂̃��C�t�^�C���̓N���C�A���g�̓s���ɍ��킹�āA�l�X�ɂȂ�܂��B���̓s�x�A�������𓮓I�m�ہA���I�J������̂������葁���g�����Ȃ̂ł����Anew�Adelete�͂���Ȃ�ɃR�X�g��������܂��B�����ŁA�\���̂̃T�C�Y���\�ߔ����Ă���̂Ȃ�A������v�[�����Ă������Ƃ������z���������v�[���ł��B
## �X���b�h�v�[���ɂ��K�{�H�������[�v�[��
�@�܊p�A�����̃p�\�R����CPU�ɕ����̃R�A������Ȃ�A�ڂ����ς��g�������A�����v�������Ƃ͖����ł��傤���H�u�����A�v���O�������������AOS������ɕ����̃R�A�Ŏd�����Ă����̂���Ȃ��́H�v�B����Ȃ킯����܂���B�uPython�����Ő���������Ă��B�v�B�������Ȑl�C���̃��C�u�������g�������ł����̂ł�������H
�@�ƁA�����킯�Ńp�\�R����CPU�����܂ł���Ԃ�v���O����������čs�����Ƃ����̂Ȃ�A�X���b�h�v�[�����g�����ɂȂ�܂��B�X���b�h�v�[���Ȃ�CPU��V�΂��Ȃ������悢�v���O�����������܂��B�������オ���OVERLAPPED�\���̂�f�[�^�ȂǁA�X���b�h�ɓn���������[�̈�̓e���|�悭��������K�v������܂��B�v�[�����Ă������\���̂���������̂Ȃ�A�e���|�悭�����ł��܂��B
## ���āA�P�b�Ԃɉ������new�o����̂��H
 �@Visual Studio 2022�i�ȉ�VS�j�ɂ́Ainstrumentation�Ƃ����v���t�@�C�����O�@�\������܂��B����́A�Ăяo�����֐��̉񐔂�ώZ���Ԃ𐳊m�ɃJ�E���g�ł���@�\�ł��B�����A�A���h�L�������g�ȂƂ��낪�����āA�֐��𐔖���Ăяo�����s���v���O���������s����ƁA�v���O�������r���ŏI�����Ă��܂��܂��B�����v���O�������g���ČĂяo���񐔂����炷����������ꍇ�A�r���ŏI�����Ȃ��̂ŁA�֐��Ăяo���񐔂Ɠr���I���͊֌W�����肻���ł��B
�@�}���`�R�A���ł̃X���b�h�v�[���̃X���[�v�b�g���v������ƁA�P�b�Ԃɐ�����̌Ăяo���͉\�ł��B�ȑO�A�؍\���̃m�[�h���������[�v�[���̃��j�b�g�ŕێ�������������̂ł����A**���b�Ԃ�1000�����j�b�g�ȏ�̉��׎g�p��**���v��������������܂����B���̂��Ƃ���A���[�N�A�C�e�����L���[�ɓ����ہA���̓s�xnew����̂͂������Ȏ����Ǝv���܂��B
## �d�g�݂͔z��̗v�f�A�h���X�̃����O�o�b�t�@
�@�ł��̂ŁA���j�b�g�A�h���X���������̔z��̒[�łȂ��ꍇ�A�O��̃A�h���X�ɂ��A�N�Z�X�o���܂����A�A�h���X��ԋp������ɂ��A�N�Z�X�o���܂��̂ŁA�g�p�҂��K���������ĉ^�p����K�v������܂��B
## �e���v���[�gstd::�R���e�i�N���X�ł��������[�v�[���͍���H
�@�͂��A���܂��B�ނ���A������̕���������������܂���B�J�X�^���A���P�[�^�[�����΁A�������[���X���b�h���[�J���Ŋm�ۂ���̂��A�v���Z�X���[�J���Ŋm�ۂ���̂��A�������[�헪�̕����L����܂��B�����A�}���`�X���b�h�Ŏg���ꍇ�A���b�N����@�\���K�v�ɂȂ�ł��傤����A���������̂������N���X����鎖�ɂȂ�ł��傤�B�������[�v�[�������n�߂����́Astd���C�u�����̎g�p�ɕs���ꂾ�������Ƃ�����A�̗p�͂��܂���ł����B
## MemoryLoan�e���v���[�g�N���X�̎g�����̃f���R�[�h
### ���̋L���Ŏg���\�[�X�R�[�h�ւ̃����N
[GitHub�ւ̃����N�͂����ł��BVisual Studio 2022�p�ɐݒ肳�ꂽsln�t�@�C��������܂��B](https://github.com/NewGoldSmith/SubProcess "https://github.com/NewGoldSmith/SubProcess")
�@TestProject���X�^�[�g�A�b�v�v���W�F�N�g�ɐݒ肵�A�\�����[�V�����G�N�X�v���[���[����**test.cpp**��I�����A�v���p�e�B�̐ݒ��**�S��->�r���h���珜�O**���ڂ�**������**�ɐݒ肵�A**test.cpp�ȊO**��**�͂�**�ɐݒ肵�A�^�[�Q�b�gCPU��x64�ɐݒ肵�A`F5`����������Ǝ��s�ł��܂��B
![](https://storage.googleapis.com/zenn-user-upload/f736f8852a36-20240623.png)

�@�\�[�X�R�[�h�̒��ɂ̓f�o�b�O�p�̃��C�u�������܂�ł��܂��B�{���ł͂Ȃ��ׁA����͐����������������܂��B

>�@���̋L���ŏЉ�Ă���\�[�X�R�[�h�́A���J�������_����ύX�������Ă��鎖������܂��B���̂��߁A���̋L���Ƃ͈قȂ錋�ʂ𓾂�ꍇ������܂��B�܂��A�\�[�X�R�[�h���@�\�ʂɃf�B���N�g���𕪂��铙�́A���P���s���\��������܂��B
 
### �f���R�[�h
 �@���L�Ƀf���R�[�h���L�ڂ��܂��B���̌�A�ԍ��̃R�����g���t�����Ă���Ƃ���́A����������s���܂��B
```test.cpp
// test.cpp
#include <iostream>
#include "../CommonLib/MemoryLoan.h"

using namespace std;

int main() {
	{
		int iArr[1]{};//1
		MemoryLoan<int> mp(iArr, sizeof(iArr) / sizeof(iArr[0]));//2
		mp.DebugString("test1");//3
		int *pi1 = mp.Lend();//4
		*pi1 = 100;//5
		cout << *pi1 << endl;//6
		mp.Return(pi1);//7
		int *pi2 = mp.Lend();//8
		*pi2 = 200;//9
		cout << *pi2 << endl;//10
		mp.Return(pi2);//11
		int iArr2[2]{};//12
		mp.ReInitialized(iArr2, sizeof(iArr2) / sizeof(iArr2[0]));//13
		mp.DebugString("test2");//14
		pi1 = mp.Lend();//15
		pi2 = mp.Lend();//16
		*pi1 = 250;//17
		*pi2 = *pi1 + 50;//18
		cout << *pi1 << endl << *pi2 << endl;//19
		mp.Return(pi1);//20
		mp.Return(pi2);//21
	}
	_CrtDumpMemoryLeaks();
}
```
#### �f���R�[�h�̉��
##### �P�A�������[�v�[���Ɏg���������[�̈���m�ۂ���
```test.cpp
		int iArr[1]{};//1
```
�@���̃������[�v�[���e���v���[�g�N���X�͎g�p���郁�����[���m�ۂ���@�\�����ڂ���Ă��܂���B���̕ӂ��std���C�u�����Ƃ͑��e��Ȃ��d�l�ł��B�P���j�b�g�m�ۂ��Ă��܂��B���̃������[�v�[���́A`int`���̃v���~�e�B�u�Ȍ^�����łȂ��A�N���X�̔z�񓙂���舵�������ł��܂��B
##### �Q�A�R���X�g���N�^�Ń������[�v�[���I�u�W�F�N�g���\�z����
```test.cpp
		MemoryLoan<int> mp(iArr, sizeof(iArr) / sizeof(iArr[0]));//2
```
�@�R���X�g���N�^�̓f�t�H���g�R���X�g���N�^���֎~�ɂ��Ă��܂��B�����`MemoryLoan(T *const pBufIn, size_t sizeIn)`���g���܂��B`pBufIn`�͔z��̃A�h���X�A`sizeIn`�͂��̔z��̗v�f���ƂȂ��Ă��܂��B
##### �R�A�f�o�b�O��������Z�b�g����
```test.cpp
		mp.DebugString("test1");//3
```
�@����͔C�ӂ̐ݒ�ł��B�����ݒ肵�Ă����ƁA�f�X�g���N�^���Ă΂ꂽ���ɁA�f�o�b�O�o�͂ɐݒ肵�������񂪕\������܂��B�����̃������[�v�[���I�u�W�F�N�g���g���Ă��鎞�ɁA�ǂ̃I�u�W�F�N�g�̃f�X�g���N�^���Ă΂ꂽ�̂��A���ʂł���悤�ɂ��Ă��܂��B
�@�܂��A���炩�̗�O�����������Ƃ����A�f�o�b�O�o�͂ɂ��̕������\������悤�ɂȂ��Ă��܂��B
##### �S�A�݂��o��������
```test.cpp
int *pi1 = mp.Lend();//4
```
�@�z��̗v�f�̃A�h���X��Ԃ��܂��B
##### �T�A�U�A���炩�̍�Ƃ�����
```test.cpp
		*pi1 = 100;//5
		cout << *pi1 << endl;//6
```
�@�����ł͑�����A�R���\�[���ɏo�͂��Ă��܂��B
##### �V�A�ԋp����
```test.cpp
		mp.Return(pi1);//7
```
�@�؂�Ă������j�b�g��ԋp���Ă��܂��B
##### �W�A�X�A�P�O�A�P�P�A�܂��؂�č�Ƃ����ĕԋp����
```test.cpp
		int *pi2 = mp.Lend();//8
		*pi2 = 200;//9
		cout << *pi2 << endl;//10
		mp.Return(pi2);//11
```
�@�܂��؂�܂��B
##### �P�Q�A�P�R�A�P�S�A�ʂ̃������[�̈���Z�b�g���čď���������B
```test.cpp
		int iArr2[2]{};//12
		mp.ReInitialized(iArr2, sizeof(iArr2) / sizeof(iArr2[0]));//13
		mp.DebugString("test2");//14
```
�@�z��̃T�C�Y��ς������̂ŁA������w�肵�āA�ď��������܂��B�ď���������ƃf�o�b�O�o�͂ɁA���O�Ɏg�p���Ă������v���f�o�b�O�o�͂ɕ\�����܂��B
##### �P�T�`�Q�P�A�V���ȗ̈�ō�Ƃ����ԋp����
```test.cpp
		pi1 = mp.Lend();//15
		pi2 = mp.Lend();//16
		*pi1 = 250;//17
		*pi2 = *pi1 + 50;//18
		cout << *pi1 << endl << *pi2 << endl;//19
		mp.Return(pi1);//20
		mp.Return(pi2);//21
```
�@���j�b�g�T�C�Y��`ReInitialized`�ő��₵���̂ŁA�V������Ƃ����Ȃ��s���Ă��܂��B
##### �R���\�[������
```�R���\�[��.����
100
200
250
300
```
## MemoryLoan�e���v���[�g�N���X���t�@�����X
### �`���ł̏����w��
�@�`���ŏ����w�肪�A�o����悤�ɂȂ��Ă��܂��B�K�v�ɉ�����`#define`���R�����g�A�E�g�A�A���R�����g���ĉ������B
```MemoryLoan.h
// ********�g�p������ݒ�***********
#define ML_USING_CRITICAL_SECTION
#define ML_CONFIRM_RANGE
#define ML_USING_DEBUG_OUT
#define ML_USING_STD_ERROR
// ******�����ݒ�I���*************
```
#### ML_USING_CRITICAL_SECTION
�@�N���e�B�J���Z�N�V�������g�p���܂��B�����̃X���b�h�œ����g�p����\��������ꍇ�A���̋@�\���g���܂��B
#### ML_CONFIRM_RANGE
�@�ݏo���ߑ��A�ԋp�ߑ��ɂȂ��Ă��Ȃ����m�F���܂��B�Ȃ��Ă����ꍇ�A��O�𓊂��܂��B
#### ML_USING_DEBUG_OUT
�@�f�X�g���N�^���Ă΂ꂽ���ɁA�f�o�b�O�o�͂ɏ����o�͂��܂��B
>MemoryLoan is destructing. DebugMessage:"test2" TypeName:"int" BytesPerUnit:4bytes TotalNumberOfLoans:2 TotalNumberOfReturns:2 NumberOfUnreturned:0 NumberOfUnits:2 MaximumNumberOfLoans:2

�̗l�Ƀf�o�b�O�o�͂���܂��B���ꂼ��̓��e�̈Ӗ��͎��̂悤�ɂȂ�܂��B
##### DebugMessage:"test2"
�@�\�߃f�o�b�O���b�Z�[�W��`DebugString`���\�b�h�ŕ�������d�g��ł����ƁA**""**�̊Ԃɂ��̕����񂪕\������܂��B�C���X�^���X�𕡐�������ꍇ�̎��ʂɂ��g���܂��B
##### TypeName:"int"
�@�^����\�����܂��B
##### BytesPerUnit:4bytes
�@�P���j�b�g�ӂ�̃������[�g�p�ʂ�\�����܂��B
##### TotalNumberOfLoans:2
�@���ݏo����\�����܂��B
##### TotalNumberOfReturns:2
�@���ԋp����\�����܂��B
##### NumberOfUnreturned:0
�@�f�X�g���N�^���Ă΂ꂽ���́A���ԋp����\�����܂��B
##### NumberOfUnits:2
�@���j�b�g����\�����܂��B
##### MaximumNumberOfLoans:2
�@�s�[�N�̍ő�ݏo������\�����܂��B
#### ML_USING_STD_ERROR
�@�G���[�o�͂�cerr�ɏo�͂��܂��B

 ***�����̏����ŕ����̃C���X�^���X���g�p�������ꍇ�ǂ�����́H***
>�@MemoryLoan.h���R�s�[���āA�Ⴄ�t�@�C�����ɂ��ĕۑ����A�N���X�������Ȃ����O�ɕύX���܂��B����ŕ����̏����Ŏg�p�ł��܂��B�X�}�[�g�ł͂Ȃ��ł����ǂ������@���v�����Ȃ��̂ł��̕��@�ɂ��Ă��܂��B������������ϐ��Ŏ����āA�؂�ւ��鎖�͏o����ł��傤���ǁA�w�b�_�����������邾�������炱��ł������ȂƎv���Ă��܂��B�N���X���̕ύX�ł����AVisual Studio 2022�i�ȉ�VS�j�Ȃ�΁A�J�[�\�����N���X���Ɏ����čs���A�L�[�{�[�h�V���[�g�J�b�g`Ctrl + r, r`�ŕύX�ł��܂��BMemoryLoan.h�̖�����
```MemoryLoan.h
#undef ML_USING_CRITICAL_SECTION
#undef ML_CONFIRM_RANGE
#undef ML_USING_DEBUG_OUT
#undef ML_USING_STD_ERROR
```
>�̗l�ɁA#define��#undef���Ă��܂��̂ŁA�e�t�@�C�����ƂɈႤ�ݒ肪�\�ł��B

### �����o�[�֐��y�сA�R�[�h���
��������̓����o�[�֐��y�сA�R�[�h����ł��B�N���X����**MemoryLoan**�ł��B

#### MemoryLoan(T *const pBufIn, size_t sizeIn)
##### ����
�@�R���X�g���N�^
##### ����
###### pBufIn
  �@�z��̃A�h���X�B
###### sizeIn
�@�z��̗v�f���B

```MemoryLoan.h
	MemoryLoan(T *const pBufIn, size_t sizeIn)
		:ppBuf(nullptr)
		, front(0)
		, end(0)
		, mask(sizeIn - 1)
	{
	// @attention sizeIn��2�ׂ̂���łȂ��Ă͂Ȃ�܂���B
		ppBuf = new T * [sizeIn];
		for (size_t i(0); i < sizeIn; ++i) {
			ppBuf[i] = &pBufIn[i];
		}
	}

```
�@�������������𔲂��o�����R�[�h�ł��B�z��̊e�v�f�̃A�h���X���i�[����z���`new`�Ŋm�ۂ��A`ppBuf`�Ɋi�[���Ă��܂��B�X���b�h���[�J���ɂ���Ȃ�΂��̕ӂ̉��P���o���܂��B�����o�[�͔z��A�h���X���i�[����`ppBuf`�A�����O�o�b�t�@�̐擪���L�^����`front`�A�������L�^����`end`�A�A�h���X���}�X�N���ē����A�h���X���O���O�����l�ɂ���ׂ́A`mask`�����������Ă��܂��B`mask`��`�z��̗v�f��-1`�ł��̂ŁA�z��̗v�f����m�肽���Ƃ��ɂ��g���܂��B**�v�f����2�ׂ̂���łȂ��ꍇ�A��������ȃo�O�ɑ�������\��**������܂��̂ŏ\�����ӂ��K�v�ł��B
#### ~MemoryLoan()
##### ����
�f�X�g���N�^�B
```MemoryLoan.h
	~MemoryLoan() {
		delete[] ppBuf;
	}
```
�@`ppBuf`��`delete`���Ă��܂��B
#### void ReInitialized(T *pBufIn, size_t sizeIn)
##### ����
�@�ď����������܂��B
##### ����
###### T *pBufIn
�@T�^�̔z��̃A�h���X�B
###### size_t sizeIn
�@�z��̃��j�b�g���B
 ```MemoryLoan.h
 	// @attention sizeIn��2�ׂ̂���łȂ��Ă͂Ȃ�܂���B
	void ReInitialized(T *pBufIn, size_t sizeIn) {
		delete[] ppBuf;
		front = 0;
		end = 0;
		mask = sizeIn - 1;
		ppBuf = new T * [sizeIn];
		for (size_t i(0); i < sizeIn; ++i) {
			ppBuf[i] = &pBufIn[i];
		}
	}
```
�@�܂��A�ď��������鎖�͖����Ǝv���܂����A`�R�s�[�R���X�g���N�^`�A`move�R���X�g���N�^`���g���Ȃ��̂ŁA����̑���ɂȂ镨��p�ӂ��܂����B�������悤�Ǝv���΂ł������ł����A���̏��A�K�v�Ȃ��̂Ŏ������Ă��܂���B���R`std::vecotr`���̗v�f�ɂ͎g���܂���B
#### T *Lend()
##### ����
�@�ݏo�������܂��B
##### �߂�l
###### T *
�@T�^�̃��j�b�g�̃|�C���^��Ԃ��܂��B
```MemoryLoan.h
	inline T *Lend() {
		T **ppT = &ppBuf[end & mask];
		++end;
		return *ppT;
	}
```
�@�ݏo�������āA`end`���C���N�������g���Ă��܂��B�����̘A�������v�f���������͏o���܂���B
#### void Return(T *const pT)
##### ����
�@�ԋp���󂯕t���܂��B
##### ����
###### T *const pT
�@T�^�̕ԋp����|�C���^�B
 ```MemoryLoan.h
	inline void Return(T *const pT) {
		ppBuf[front & mask] = pT;
		++front;
	}
```
�@�ԋp������`front`���C���N�������g���Ă��܂��B
#### void DebugString(const std::string &str)
##### ����
�@�f�o�b�O�p�̕������ݒ肵�܂��B
##### ����
###### const std::string &str
�@�ݒ肷�镶����B
  ```MemoryLoan.h
	void DebugString(const std::string &str) {
		strDebug = str;
	}
```
�@�f�X�g���N�^���Ă΂ꂽ���A�f�o�b�O�o�͂ɕ�������o�͂��܂��B�ǂ̃I�u�W�F�N�g�̃f�X�g���N�^���Ă΂ꂽ���A���ʉ\�ɂ��鎖���o���܂��B
## �ȏ�A���t�@�����X�I���

>�@SubProcess�N���X�́A�@�\����ׁ̈A���ǂ������鎖������܂��B�܂��A�ǉ��̃����o�[����������\��������܂��B

# �I����
�@�u[C++]�������e���v���[�g�������[�v�[���R���e�i�̎����v�̉���͈ȏ�ƂȂ�܂��B���̋L�����F�l�̑M���┭�z�̂��������ɂȂ�܂�����K���ł��B
 �@�܂��A���ӌ��A�����z�A������ȂǁA���҂����Ă���܂��B

