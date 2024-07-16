#pragma once
#include <crtdbg.h>
#include "../SubProcess/SubProcess2.h"
#include "../CommonLib/defSTRINGIZE.h"

#pragma comment(lib,  "../SubProcess/" STRINGIZE($CONFIGURATION) "/SubProcess-" STRINGIZE($CONFIGURATION) ".lib")
//#pragma comment(linker,"/STACK:10000000")
