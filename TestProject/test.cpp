// test.cpp
#include <iostream>
#include "../CommonLib/MemoryLoan.h"

using namespace std;

int main() {
	{
		int iArr[1]{};
		MemoryLoan<int> mp(iArr, sizeof(iArr) / sizeof(iArr[0]));
		mp.DebugString("test1");
		int *pi1 = mp.Lend();
		*pi1 = 100;
		cout << *pi1 << endl;
		mp.Return(pi1);
		int *pi2 = mp.Lend();
		*pi2 = 200;
		cout << *pi2 << endl;
		mp.Return(pi2);
		int iArr2[2]{};
		mp.ReInitialized(iArr2, sizeof(iArr2) / sizeof(iArr2[0]));
		mp.DebugString("test2");
		pi1 = mp.Lend();
		pi2 = mp.Lend();
		*pi1 = 250;
		*pi2 = *pi1 + 50;
		cout << *pi1 << endl << *pi2 << endl;
		mp.Return(pi1);
		mp.Return(pi2);
	}
	_CrtDumpMemoryLeaks();
}
