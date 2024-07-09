// test2.cpp
#include "./test2.h"
using namespace std;

int main() {
   {
		string str;											// １
		SubProcess sp;
		sp.SetUseStdErr(true);							// ２
		if( !sp.Popen(R"(cmd.exe)") )
			return 1;
		if( !(sp.Await(1000) >> cout) )
			return 1;

 
      if (!(sp >> cout)) {								// ３
         debug_fnc::ENOut(sp.GetLastError());
         sp.ResetFlag();
      }

      if (!(sp << "chcp" << endl))					// ４
         return 1;
      if (!(sp >> str))
         return 1;
      cout << str;

      if (!(sp << "chcp" << "\n"))					// ５
         return 1;
      for (; sp.IsReadable();) {
         if( !(sp >> cout) )
            return 1;
      }

      if (!(sp << "chcp"))								// ６
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
      cout << "\nPlease enter the code page number." << endl;
      if (!(sp << cin))
         return 1;
      if (!(sp >> cout))
         return 1;
      if (sp.CErr().IsReadable()) {
         if (!(sp.CErr() >> cerr))
            return 1;
      }

      if (!(sp << "exit" << endl))					// ８
         return 1;
      for (; sp.IsReadable();) {
         if(sp.Await(1000) >> cout)
            return 1;
      }

      SleepEx(0, NULL);

      if (!(sp.WaitForTermination(INFINITE)))	// ９
         return 1;

      DWORD dw = sp.GetExitCodeSubProcess();		// １０
      cout << "\nExit code is " << dw << endl;

      sp.Pclose();										// １１

      cout << "\nThe SubProcess demo has successfully concluded." << endl;
   }
   _CrtDumpMemoryLeaks();
}
