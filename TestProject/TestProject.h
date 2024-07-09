#pragma once
#include <string>
#include <string_view>
#include <conio.h>
#include <memory>
#include <yvals_core.h>
#include "../Debug_fnc/debug_fnc.h"
#include "../Evaluate/evaluate.hpp"
#include "../Yamanas/IdxFnc.h"
#include "../BitManip/mm256_custom.h"
//#include "../Engine/"
#pragma comment(lib,  "../BitManip/" _CRT_STRINGIZE($CONFIGURATION) "/BitManip-" _CRT_STRINGIZE($CONFIGURATION) ".lib")
#pragma comment(lib,  "../Debug_fnc/" _CRT_STRINGIZE($CONFIGURATION) "/Debug_fnc-" _CRT_STRINGIZE($CONFIGURATION) ".lib")
#pragma comment(lib,  "../Engine/" _CRT_STRINGIZE($CONFIGURATION) "/Engine-" _CRT_STRINGIZE($CONFIGURATION) ".lib")
#pragma comment(lib,  "../Evaluate/" _CRT_STRINGIZE($CONFIGURATION) "/Evaluate-" _CRT_STRINGIZE($CONFIGURATION) ".lib")
#pragma comment(lib,  "../Yamanas/" _CRT_STRINGIZE($CONFIGURATION) "/Yamanas-" _CRT_STRINGIZE($CONFIGURATION) ".lib")

