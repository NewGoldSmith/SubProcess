/**
 * @file testIOCP.cpp
 * @brief IOCPテスト実装
 * SPDX-License-Identifier: MIT<br>
 * @date 2024<br>
 * @author Gold Smith
 */
#include "testIOCP.h"
using namespace std;
int main() {
	{
		sampleIOCP s;// 1
		if (!(s << "Hello, World!"))// 2
			return 1;
		string str;
		if (!(s >> str))// 3
			return 1;
		s.OrCout.Push("Main:\"" + str + "\"");// 4
	}
	_CrtDumpMemoryLeaks();
	return 0;
}
