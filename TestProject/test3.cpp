// test3.cpp
#include "test2.h"
using namespace std;

int main() {
	{
		string str;										// １
		SubProcess sp;
		if (!sp.Popen(R"(cmd.exe)"))
			return 1;

		sp.SleepEx(2000);								// ２
		
		for (; sp.IsActive();) {					// ３
			if (sp.IsReadable()) {
				if (!(sp >> cout))
					return 1;
				else
					sp.SleepEx(10);
					continue;

			} else {
				if (!(sp << cin))
					return 1;
				sp.SleepEx(10);
			}
		}

		if (!(sp.WaitForTermination(INFINITE)))// ４
			return 1;

		DWORD dw = sp.GetExitCodeSubProcess();	// ５
		cout << "\nExit code is " << dw << endl;

		sp.Pclose();									// ６
	}
	_CrtDumpMemoryLeaks();

}
