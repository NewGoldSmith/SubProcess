// test.h
#pragma once
#include <Windows.h>
#include <string>
#include "../Debug_fnc/debug_fnc.h"
#include "../SubProcess/SubProcess.h"
#define STRINGIZE__(x) #x
#define STRINGIZE(x) STRINGIZE__(x)

#pragma comment(lib,  "../SubProcess/" STRINGIZE($CONFIGURATION) "/SubProcess-" STRINGIZE($CONFIGURATION) ".lib")
#pragma comment(linker,  "/STACK:1000000")
