/**
 * @file testIOCP.cpp
 * @brief IOCPテスト実装
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#include "testIOCP.h"
#include <conio.h>
using namespace std;
int main() {
	{
		sampleIOCP s;// 1
		if (!(s << "Hello, World!"))// 2
			return 1;
		string str;
		if (!(s.Await(100) >> str))// 3
			return 1;
		s.OrCout.Push("Main:\"" + str + "\"");// 4

		s.OrCout.StopTimer();// 5
		s.OrCout.ShowTimeDisplay(false);// 6
		stringstream ss;// 7
		ss << std::fixed << std::setprecision(3) << s.OrCout.TotalTime() << " msec";
		s.OrCout.Push("Main:total elapsed time. " + ss.str() );
	}
	_CrtDumpMemoryLeaks();
	(void)_getch();
	return 0;
}
