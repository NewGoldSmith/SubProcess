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
