// test.cpp
#include <iostream>
#include <vector>
#include "MemoryLoan.h"
#include "./stack_resource.h"

using namespace std;

int main() {
	{
		cc30::stack_resource<100> s{};
		std::pmr::memory_resource *mr = &s;

		int iArr[1]{};
		MemoryLoan<int, std::pmr::polymorphic_allocator<int *>>
			mp(iArr, sizeof(iArr) / sizeof(iArr[0]), mr);
		mp.DebugString("test1");
		int *pi1 = mp.Lend();
		*pi1 = 100;
		cout << *pi1 << endl;
		mp.Return(pi1);
		int *pi2 = mp.Lend();
		cout << *pi2 << endl;
		int iArr2[2]{};
		mp.ReInitialized(iArr2, sizeof(iArr2) / sizeof(iArr2[0]));
		mp.DebugString("test2");
		pi1 = mp.Lend();
		pi2 = mp.Lend();
		*pi1 = 250;
		*pi2 = *pi1 + 50;
		cout << *pi1 << " " << *pi2 << endl;
		mp.Return(pi1);
		mp.Return(pi2);
	}
	_CrtDumpMemoryLeaks();

}
