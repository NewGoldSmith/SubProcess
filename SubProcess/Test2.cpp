// test2.cpp
#include <crtdbg.h>
#include "./SubProcess.h"
using namespace std;

int main() {
   {
      string str;												// １
      SubProcess sp;
      if (!sp.Popen(R"(cmd.exe)"))
         return 1;
      if (!(sp.Await(3000) >> cout))
         return 1;

      sp.SetUseStdErr(true);									// ２

      if (!(sp >> cout)) {									// ３
         DWORD result = sp.GetLastError();
         cout << "\n\nError code is " << result << endl;
         sp.ResetFlag();
      }

      if (!(sp << "chcp" << endl))							// ４
         return 1;
      if (!(sp >> str))
         return 1;
      cout << str;

      if (!(sp << "chcp" << "\n"))							// ５
         return 1;
      for (; sp.IsReadable();) {
         sp >> cout;
      }

      if (!(sp << "chcp"))									// ６
         return 1;
      if (sp.IsReadable()) {
         if (!(sp >> cout))
            return 1;
      }
      if (!sp.Flush())
         return 1;
      if (sp.IsReadable()) {
         if (!(sp >> cout))
            return 1;
      }
      if (!(sp << endl))
         return 1;
      if (sp.IsReadable()) {
         if (!(sp >> cout))
            return 1;
      }

      if (!(sp << "chcp "))							// ７
         return 1;
      cout << "\n\nPlease enter the code page number." << endl;
      if (!(sp << cin))
         return 1;
      if (!(sp >> cout))
         return 1;
      if (sp.CErr().IsReadable()) {
         if (!(sp.CErr() >> cerr))
            return 1;
      }

      if (!(sp << "exit" << endl))						// ８
         return 1;
      for (; sp.IsReadable();) {
         sp.Await(1000) >> cout;
         if (!sp.SleepEx(100))
            return 1;
      }

      if (!(sp.WaitForTermination(INFINITE)))			    // ９
         return 1;

      DWORD dw = sp.GetExitCodeSubProcess();				// １０
      cout << "\nExit code is " << dw << endl;

      sp.Pclose();										// １１

      cout << "\nThe SubProcess demo has successfully concluded." << endl;
   }
   _CrtDumpMemoryLeaks();
}
