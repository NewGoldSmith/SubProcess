#include "TestProject.h"
#include "../BitManip/m256_manip.h"
#include <conio.h>


using namespace std;
using namespace bit_manip;
using namespace debug_fnc;
using namespace Evaluate;

#define p1 0b\
00000000\
00000000\
00000000\
00001000\
00010000\
00000000\
00000000\
00000000ULL
#define o1 0b\
00000000\
00000000\
00000000\
00010000\
00001000\
00000000\
00000000\
00000000ULL
#define m1 0b\
00000000\
00000000\
00010000\
00000000\
00000000\
00000000\
00000000\
00000000ULL

constexpr std::string_view get_data() {
	return "Check out this data string.";
}
int main() {
	try {
		const unique_ptr<TP_POOL, decltype(CloseThreadpool) *>ptpp = { []() {
			PTP_POOL ptpp = CreateThreadpool(NULL);
			if (!ptpp) {
				throw runtime_error(EOut);
			}
			SetThreadpoolThreadMaximum(ptpp, 6);
			return ptpp; }(),CloseThreadpool };

		const unique_ptr<TP_CALLBACK_ENVIRON, void(*)(TP_CALLBACK_ENVIRON *)>pcbe{ [](PTP_POOL pt) {
				const auto pcbe1 = new TP_CALLBACK_ENVIRON;
				InitializeThreadpoolEnvironment(pcbe1);
				SetThreadpoolCallbackPool(pcbe1, pt);
				SetThreadpoolCallbackRunsLong(pcbe1);
				return pcbe1;
			}(ptpp.get())
				, [](PTP_CALLBACK_ENVIRON pcbe) {DestroyThreadpoolEnvironment(pcbe); delete pcbe; } };

		PTP_CLEANUP_GROUP_CANCEL_CALLBACK const pfng{ [](
				 _Inout_opt_ PVOID ObjectContext,
				 _Inout_opt_ PVOID CleanupContext) {
				if (!ObjectContext) {
					dout("Context is NULL.");
					return;
				} else {
					void *pnode = ObjectContext;
				}
				if (!CleanupContext) {
				} else {
					void *pnode = CleanupContext;
				}
			}
		};

		const unique_ptr<TP_CLEANUP_GROUP, void(*)(PTP_CLEANUP_GROUP)>ptpcg
		{ [](PTP_CALLBACK_ENVIRON pcbe,PTP_CLEANUP_GROUP_CANCEL_CALLBACK pfn) {
			PTP_CLEANUP_GROUP const ptpcg = CreateThreadpoolCleanupGroup();
			SetThreadpoolCallbackCleanupGroup(pcbe, ptpcg, pfn);
			return ptpcg; }(pcbe.get(),pfng)
		,	[](PTP_CLEANUP_GROUP ptpcg) {
			CloseThreadpoolCleanupGroupMembers(ptpcg,FALSE,(void *)-1);
			CloseThreadpoolCleanupGroup(ptpcg);	}
		};
		[[maybe_unused]] constexpr auto sv = get_data();
		std::cout << sv << std::endl;
		(void)_getch();
	} catch (std::exception &e) {
		_D(e.what());
		MessageBoxA(NULL, (std::string("error.") + e.what()).c_str(), "error", MB_ICONEXCLAMATION);
	}
	_CrtDumpMemoryLeaks();
	return 0;
}
