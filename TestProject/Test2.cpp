// test2.cpp
#include "./test2.h"
using namespace std;

int main() {
   {
		string str;											
		SubProcess sp;
		sp.SetUseStdErr(true);  // １
		if( !sp.Popen(R"(cmd.exe)") ) // ２
			return 1;
		if( !(sp.Await(1000) >> cout) ) // ３
			return 1;

 
      if (!(sp >> cout)) { 	// ４
         debug_fnc::ENOut(sp.GetLastError());
         sp.ResetFlag();
      }

      if (!(sp << "chcp" << endl))	// ５
         return 1;
      if (!(sp >> str))
         return 1;
      cout << str;

      if (!(sp << "chcp" << "\n"))	// ６
         return 1;
      for (; sp.IsReadable(100);) {
         if( !(sp >> cout) )
            return 1;
      }

      if (!(sp << "chcp"))	// ７
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
      for( ; sp.IsReadable(100); ){
         if (!(sp >> cout))
            return 1;
      }

      if (!(sp << "chcp "))// ８
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

      if (!(sp << "exit" << endl))	// ９
         return 1;
      if(sp.IsReadable(100)) {
         sp >> cout;
      }

      if (!(sp.WaitForTermination(INFINITE))) // １０
         return 1;

      DWORD dw = sp.GetExitCodeSubProcess(); // １１
      cout << "\nExit code is " << dw << endl;

      sp.Pclose(); // １１

      cout << "\nThe SubProcess demo has successfully concluded." << endl;
   }
   _CrtDumpMemoryLeaks();
}
