# この記事の対象読者
　**Python**の**subprocess**モジュールの使われ方を見たとき、「えっ、こんな手軽な使い方が出来るんだ！」と、感じた事はないでしょうか？C++でも手軽に子プロセスを使いたい、そんな方向けの記事です。Windows固有の非同期IOのアーキテクテャを使っていますので、他のOS向けへの移植は出来ません。`OVERLAPPED`構造体、`ReadFileEx`、`WriteFileEx`など、stdライブラリより下層のAPIを使い、**シングルスレッド**、**ノンプリエンティブマルチタスク**向けの、古風な記法で書かれています。
　また、このプログラムを使ったSSHの接続の仕方の記事を、次回以降に紹介したいと思っています。
　そんなニッチなプログラム技法に、興味がある方向けの記事です。
 また、このクラスの実装には、

https://qiita.com/GoldSmith/items/1de197c10c05c054461a 

の、メモリープールの技術を使って、OVERLAPPED構造体を使いまわしています。こちらの記事も、よろしければご覧ください。

##  Python subprocess.popenは、子プロセスとのパイプ通信が手軽に可能。
　Windowsでも`_popen`があるじゃないか！と、思われるかもしれませんが、Microsoftドキュメントによると、
 >Windows プログラムで使用すると、_popen 関数は無効なファイル ポインターを返し、その結果、プログラムは無期限に応答を停止します。 _popen は、コンソール アプリケーションで正しく動作します。 入力と出力をリダイレクトする Windows アプリケーションを作成するには、Windows SDK でリダイレクトされた入力と出力を使用した子プロセスの作成を参照してください。 

　と、なっています。もしかしたら、他のプログラムと連携した、標準入出力で通信するGUIアプリケーションを作る可能性もあるので、これでは不安です。Pythonでは**分け隔てなく**使えるようです。というわけで、[リダイレクトされた入出力を使用した子プロセスの作成](https://learn.microsoft.com/ja-jp/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output "リダイレクトされた入出力を使用した子プロセスの作成")を参照して、**SubProcess**クラスを作ってみました。
## SubProcessクラスの使い方のデモコード
### この記事で使うソースコードへのリンク
[GitHubへのリンクはここです。Visual Studio 2022用に設定されたslnファイルもあります。](https://github.com/NewGoldSmith/SubProcess "https://github.com/NewGoldSmith/SubProcess")
　TestProjectをスタートアッププロジェクトに設定し、ソリューションエクスプローラーから**test2.cpp**を選択し、プロパティの設定で**全般->ビルドから除外**項目を**いいえ**に設定し、**test2.cpp以外**は**はい**に設定し、ターゲットCPUをx64に設定し、`F5`を押下すると実行できます。
　ソースコードの中にはデバッグ用のライブラリも含んでいます。本質ではない為、今回は説明を割愛いたします。

:::note info
 　この記事で紹介しているソースコードは、公開した時点から変更を加えている事があります。そのため、元の記事とは異なる結果を得る場合があります。また、ソースコードを機能別にディレクトリを分ける等の、改善を行う可能性があります。
:::
 
### デモコード
 　下記にデモコードを記載します。その後、番号のコメントが付けられているところの、解説を順次していきます。
```test2.cpp
// test2.cpp
#include <crtdbg.h>
#include "test2.h"
using namespace std;

int main() {
   {
      string str;										// １
      SubProcess sp;
      if (!sp.Popen(R"(cmd.exe)"))
         return 1;
      if (!(sp.Await(3000) >> cout))
         return 1;

      sp.SetUseStdErr(true);						// ２

      if (!(sp >> cout)) {							// ３
         DWORD result = sp.GetLastError();
         cout << "\n\nError code is " << result << endl;
         sp.ResetFlag();
      }

      if (!(sp << "chcp" << endl))				// ４
         return 1;
      if (!(sp >> str))
         return 1;
      cout << str;

      if (!(sp << "chcp" << "\n"))				// ５
         return 1;
      for (; sp.IsReadable();) {
         sp >> cout;
      }

      if (!(sp << "chcp"))							// ６
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

      if (!(sp << "chcp "))						// ７
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

      if (!(sp << "exit" << endl))				// ８
         return 1;
      for (; sp.IsReadable();) {
         sp.Await(1000) >> cout;
         if (!sp.SleepEx(100))
            return 1;
      }

      if (!(sp.WaitForTermination(INFINITE)))// ９
         return 1;

      DWORD dw = sp.GetExitCodeSubProcess();	// １０
      cout << "\nExit code is " << dw << endl;

      sp.Pclose();									// １１

      cout << "\nThe SubProcess demo has successfully concluded." << endl;
   }
   _CrtDumpMemoryLeaks();
}
```
### １、`cmd.exe`を子プロセスとして起動
 ```test2.cpp
		string str;												// １
		SubProcess sp;
		if (!sp.Popen(R"(cmd.exe)"))
			return 1;
		if (!(sp.Await(3000) >> cout))
			return 1;
 ```
#### sp.Popen(R"(cmd.exe)")
　子プロセスとしてcmd.exeを起動。
#### sp.Await(3000) >> cout
　cmd.exeは起動すると文字列を出力する事が予め判っているので、それを受け取り`std::cout`に出力。`Await(3000)`は３秒間待つという事です。非同期ですので、子プロセスが起動してから文字出力までを、受け取る側が**待つ**という動作をしています。それ以上かかる場合、**タイムアウトエラー**(エラーコード：**WAIT_TIMEOUT**）として先へ進むようになっています。タイムアウトしないと、先に進めないのでこのような仕様にしています。
### ２、標準エラー出力を分離して出力する。
```test2.cpp
  	sp.SetUseStdErr(true);									// ２
```
　子プロセスが、標準エラーで出力した物を標準エラーで受け取るか、どうか設定します。
### ３、わざとタイムアウトエラーにさせ、エラーコードを取得
```test2.cpp
		if (!(sp >> cout)) {									// ３
			DWORD result = sp.GetLastError();
			cout << "\n\nError code is " << result << endl;
			sp.ResetFlag();
		}
```
#### sp >> cout
　子プロセスのcmd.exeは親プロセスがコマンドを入力するまで何も出力しません。出力しないものを待っているのでタイムアウトエラーになります。エラーコードは`WAIT_TIMEOUT`でwinerror.hで定義されています。他のエラーも前出のヘッダーで定義されています。
#### DWORD result = sp.GetLastError();
　最後のエラーコードを取得します。
#### sp.ResetFlag();
　内部のエラーフラグをリセットします。これをしないと次のメンバー関数が実行できない仕様になっています。
### ４、コマンドを送信、その後、返信を受け取る
```test2.cpp
		if (!(sp << "chcp" << endl))							// ４
			return 1;
		if (!(sp >> str))
			return 1;
		cout << str;
```
#### sp << "chcp" << endl
　これで子プロセスにコマンドを送ります。"**chcp**"は、コンソールの文字コードを表示したり変更したりするコマンドです。`endl`はストリームの最後に'\n'を加えてストリームフラッシュする`std::endl`をそのまま使っています。
#### sp >> str;、cout << str;
　そのコマンドは必ず文字を返すことが判っているので、`std::string`で受け取り、表示をします。
### ５、std::endlしないで、改行までを送信したらどうなる？
```test2.cpp
		if (!(sp << "chcp" << "\n"))							// ５
			return 1;
		for (; sp.IsReadable();){
			sp >> cout;
		}
```
#### sp << "chcp" << "\n"
　実は改行文字を入力すると、子プロセスに送信されます。改行文字を入力しないとバッファにデータが入ったままで送信されません。これは、SubProcess内部の仕様でそうしています。
#### sp.IsReadable();
　子プロセスから出力を読み込めるかどうかを、聞くことが出来ます。
### ６、Flushして送信しても、cmd.exeは改行が無いと仕事をしない事を確認
```test2.cpp
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
```
#### sp << "chcp"
　改行無しでコマンドを入力しました。
#### sp.IsReadable()
　この時点では、レスポンスが無い事を確認。
#### sp.Flush()
　バッファの内容を送りました。
#### sp.IsReadable()
　レスポンスがあるか確認。この時は無し。
#### sp << endl
　`std::endl`を流し込みました。`flush`され、改行が送られました。
#### sp.IsReadable()
　読み込めることを確認。`sp >> cout`でコンソールに出力。
### ７、std::cinから入力されたデータをダイレクトに子プロセスに流す
```test2.cpp
    	if (!(sp << "chcp " ))							// ７
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
```
#### sp << "chcp "
　`"chcp"+スペース`を入れた文字列を流し込みます。
#### cout << "\n\nPlease enter the code page number." << endl;
　コードページの入力を促す文字列を出力します。
#### sp << cin
　`std::cin`を直接流し込みます。ここで、プログラムはコンソールからの入力待ちになり、スレッドがブロッキングされてしまいます。`std::cin`の使用にはこの事を考慮する必要があります。コンソールからコードページを入力し`Enter`を押してください。
```consol.out
Please enter the code page number.
932
現在のコード ページ: 932
```
このような表示になるでしょう。
#### sp.CErr().IsReadable()
　子プロセスから親プロセスの`std::cerr`へ向けての出力が無いか確認しています。まずは無いでしょう。
#### sp.CErr() >> cerr
　もしあれば標準エラーに、出力するようにしています。
### ８、cmd.exeへ"exit"の送信
```test2.cpp
		if (!(sp << "exit" << endl))						// ８
			return 1;
		for (; sp.IsReadable();) {
			sp.Await(1000) >> cout;
			if (!sp.SleepEx(100))
				return 1;
		}
```
#### sp << "exit" << endl
　"exit"コマンドを送信します。
#### for (; sp.IsReadable();)
　データが送られ続ける間は、`for`ループで出力します。
#### sp.SleepEx(100)
　`SleepEx`は何か送られてこないか待ちます。送られてくれば0.1秒待たずに待ちを終了します。タイムアウトしてもエラー記録を残しません。
### ９、子プロセスのクリーンナップを待つ
```test2.cpp
     if (!(sp.WaitForTermination(INFINITE)))			    // ９
			return 1;
```
#### sp.WaitForTermination(INFINITE)
　一般に"exit"を入力するとcmd.exeは、終了する事が知られています。しかし、終了処理のクリーンナップに時間が、かかる事があります。終了するまで親スレッドをブロックしてcmd.exeが終了するのを待ちます。`INFINITE`の部分は時間を1000分の1秒単位で設定できます。その時間が過ぎても終了処理が終わらない場合は、タイムアウトエラーを記録し`false`を返します。`INFINITE`はwinbase.hで定義されています。
　もし、強制的に終了させなければならない事態になった場合は、`bool TerminateProcess(DWORD dw)`を使うと終了させることが出来ます。このメンバー関数はWindowsAPIの`TerminateProcess`を使っています。Microsoftのドキュメントによれば、
 >TerminateProcess 関数は、プロセスを無条件に終了させるために使用されます。 ExitProcess ではなく TerminateProcess を使用すると、ダイナミック リンク ライブラリ (DLL) によって保持されるグローバル データの状態が損なわれる可能性があります。

となっていますので、DLLを使う場合には扱いに注意が必要そうです。
### １０、終了コードを取得する
```test2.cpp
		DWORD dw = sp.GetExitCodeSubProcess();				// １０
		cout << "\nExit code is " << dw << endl;
```
#### sp.GetExitCodeSubProcess()
　子プロセスの終了コードを取得します。その結果で次のアクションへの分岐も出来ます。
### １１、子プロセスの操作ハンドルの使用を終了させる
```test2.cpp
		sp.Pclose();										// １１
```
#### sp.Pclose()
子プロセスを操作できるハンドルを終了させます。この後、新たに子プロセスを起動させる事が出来ます。
## 以上、デモコードの解説終了
　cmd.exeの挙動に沿った解説でした。違う子プロセスを扱うには、**事前に子プロセスの挙動を理解している必要**があります。プログラムによっては、起動直後、何のメッセージを返さないプログラムもあります。

:::note info
　SubProcessの子プロセスとして動作する`cmd.exe`は、**通常のコマンドプロンプトで実行できる全てのコマンドが、実行できるわけではありません。**
　コンソールアプリは、**`conhost.exe`というプログラムにホストされて、Windows上にコンソールウィンドウを表示して、実行しています。** コンソールアプリは、コンソールウィンドウを表示するコードを持っていない為、このような仕様になっています。おそらくこの為に、全てのコマンドが実行できないものと思われます。Windowsには、**擬似コンソール**という**ユーザープログラムが、`conhost.exe`の代わりを担う仕組み** が用意されています。この仕組みを使うと子プロセスの`cmd.exe`は、フルに機能を発揮出来ます。
:::
 
## SubProcessリファレンス
　ここからはSubProcessクラスの解説です。

:::note info
　SubProcessクラスは、機能向上の為、改良を加える事があります。また、追加のメンバーが加えられる可能性もあります。
:::
 
### SubProcess()noexcept
#### 説明
　コンストラクタ。
### ~SubProcess()
#### 説明
　デストラクタ。
### void ResetFlag()noexcept
#### 説明
　内部のエラー保持変数をリセットします。エラー保持変数が０以外の場合、ほとんどのメンバー関数の実行が失敗に終わります。
### DWORD SetTimeOut(DWORD uiTime)noexcept
#### 説明
　タイムアウトになるタイムを設定します。
#### 引数
##### DWORD uiTime
　タイムアウトになる時間を1000分の1秒単位で指定します。
#### 戻り値
　以前に設定されていた時間。
### bool IsTimeOut()const noexcept
#### 説明
　直前のタイムアウトになる可能性のある操作がタイムアウトになっていたか出力します。
#### 戻り値
　タイムアウトになっていたら`true`、それ以外は`false`。
### bool Popen(const std::string &strCommand)
#### 説明
　子プロセスを起動させます。
#### 引数
##### const std::string &strCommand
　起動させる文字列を指定します。
#### 戻り値
　成功したら`true`、それ以外は`false`。
### bool Pclose()
#### 説明
　子プロセスの操作ハンドルを閉じます。これを閉じるとexit コードの取得は出来なくなります。
#### 戻り値
　成功したら`true`、それ以外は`false`。
### DWORD GetExitCodeSubProcess()
#### 説明
　子プロセスの終了コードを返します。子プロセスが終了する前にこの関数が呼ばれて成功した場合、`STILL_ACTIVE`が返されます。失敗した場合は、０を返します。`DWORD GetLastError()`でエラーコードを取得できます。子プロセスが終了した後、この関数が呼ばれて成功した場合、終了コードが返されます。`STILL_ACTIVE`はminwinbase.hで定義されています。
#### 戻り値
　子プロセスの終了コード。
### bool TerminateProcess(DWORD dw)
#### 説明
　子プロセスとそのすべてのスレッドを終了します。

:::note info
　TerminateProcess 関数は、プロセスを無条件に、終了させるために使用されます。 ExitProcess ではなく TerminateProcess を使用すると、ダイナミック リンク ライブラリ (DLL) によって保持されるグローバル データの状態が、損なわれる可能性があります。
:::
#### 引数
##### DWORD dw
　この呼び出しの結果として終了した子プロセスによって使用される終了コードを指定します。 終了コードは、`GetExitCodeSubProcess` 関数を使用して、取得出来ます。
#### 戻り値
　成功したら`true`、それ以外は`false`。`GetLastError()`でエラーコードを取得可能。
### bool WaitForTermination(DWORD time)
#### 説明
　子プロセスの終了を待ちます。
#### 引数
##### DWORD time
　待つ許容できる時間を、ミリ秒単位で設定します。
#### 戻り値
　関数が成功したら`true`、それ以外は`false`。`GetLastError()`でエラーコードを取得できます。`WAIT_TIMEOUT`か`WAIT_FAILED`が取得できます。詳細なエラー情報を得る事は出来ません。
### DWORD GetLastError()const noexcept
#### 説明
　最後のエラーコードを取得します。
#### 戻り値
　エラーコードを返します。
### explicit operator bool() const noexcept
#### 説明
　最後のオペレーションが成功したかどうかを返します。オペレーションが失敗していた場合、`ResetFlag`が実行されるまで、次のオペレーションは実行されません。
#### 戻り値
　最後のオペレーションのエラーコードが０以外なら`false`を返します。
#### bool IsActive()
#### 説明
　子プロセスがアクティブかどうかを調べます。
#### 戻り値
　アクティブなら`true`、それ以外は`false`。
### bool IsReadable()
#### 説明
　ストリームが読み込み可能かどうかを調べます。
#### 戻り値
　読み込み可能ならば`true`、それ以外は`false`。
### bool SetUseStdErr(bool is_use)
#### 説明
　子プロセスから標準エラーの書き込みを、標準出力とは別の標準エラーとして、受け取るかどうかを設定します。
#### 引数
##### bool is_use
　独立させるなら`true`、それ以外は`false`。
#### 戻り値
　直前に設定されていたかどうかを返します。
### SubProcess &operator<<(const std::string &str)
#### 説明
　std::stringを受け付け、それを子プロセスの標準入力に出力します。この関数を実行して、エラーがあったかどうかは、`explicit operator bool() const noexcept`等の他の関数で調べる事が出来ます。
#### 引数
##### std::string str
　子プロセスに渡すデータ。テキストデータに限定しません。
##### 戻り値
　SubProcessオブジェクト。
### SubProcess &operator<< (std::istream &is)
#### 説明
　std::cinを受け付け、それを子プロセスの標準入力に出力します。この、関数が使われたスレッドは入力が完了するまで（std::getlineがブロック解除されるまで）ブロックされます。
#### 引数
#### std::istream is
　std::cin等のオブジェクト。
#### 戻り値
　SubProcessオブジェクト。
### SubProcess &operator<<(std::ostream &(*const manipulator)(std::ostream &))
#### 説明
　std::endl等のマニピュレータを受け取り操作します。
#### 引数
##### std::ostream &(*const manipulator)(std::ostream &)
　std::endl等のポインタ。
#### 戻り値
　SubProcessオブジェクト。
### SubProcess &operator<<(std::ios_base &(*const manipulator)(std::ios_base &))
#### 説明
　std::hex等のマニピュレータを受け取り操作します。
#### 引数
##### std::ios_base &(*const manipulator)(std::ios_base &)
　std::hex等のポインタ。
#### 戻り値
　SubProcessオブジェクト。
### SubProcess &SleepEx(DWORD num)
#### 説明
　numで指定した時間を最大として、ブロッキングをしてIOの完了を待ちます。指定された時間前にIO操作が完了した場合、ブロッキングは解除されます。タイムアウトになってもそれはエラーとして記録されません。他の要因でエラーになった場合、内部変数に記録され、次からのオペレーションは失敗します。エラーコードは`GetLastError()`で取得できます。
#### 引数
##### DWORD num
　ミリ秒単位で待つ時間を指定します。
#### 戻り値
　SubProcessオブジェクト。
### SubProcess &Flush()
#### 説明
　内部バッファのデータを強制的に子プロセスへ送信します。通常、`std::endl`や`'\n'`が無いと内部バッファにデータが溜められます。データの末尾に改行コードを使わない、バイナリデータの送信に使います。受信は`SubProcess &operator>>(std::string &str)`により、常にバイナリとして受信しています。
#### 戻り値
　SubProcessオブジェクト。
### SubProcess &ClearBuffer()
#### 説明
　内部の読み込み済みバッファー、書き込み前バッファー共にクリアします。
#### 戻り値
　SubProcessオブジェクト。
### SubProcess &operator>>(std::string &str)
#### 説明
　子プロセスが送信できるかスレッドを渡して確認します。読み込み済みバッファーが空でない場合、内容をstrに書き込みます。バッファーが空の場合、タイムアウト値まで待ちます。それでも子プロセスからの入力が無い場合、タイムアウトになります。
#### 引数
##### std::string &str
　バッファーの内容を書き込むstd::stringオブジェクト。
#### 戻り値
　SubProcessオブジェクト。
### SubProcess &operator>>(std::ostream &os);
#### 説明
　std::cout等のstd::ostreamにデータを流します。
#### 引数
##### std::ostream &os
　std::cout等のオブジェクト。
#### 戻り値
　SubProcessオブジェクト。
### friend std::ostream &operator<<(std::ostream &os, SubProcess &sp)
#### 説明
　std::cout等のstd::ostreamにデータを流します。
#### 引数
##### std::ostream &os
　std::cout等のオブジェクト。
##### SubProcess &sp
　SubProcessオブジェクト。
#### 戻り値
　std::cout等のオブジェクト。
### SubProcess &Await(DWORD numAwaitTime)
#### 説明
　次のストリームへの出力オペレーションの、タイムアウト値を設定します。内部のタイムアウト設定値より優先されます。１回出力するごとに、内部のタイムアウト設定値に戻されます。
#### 引数
##### DWORD numAwaitTime
　ミリ秒単位でのタイムアウト値。
#### 戻り値
　SubProcessオブジェクト。
### SubProcess &CErr()
#### 説明
　次のストリームへの出力オペレーションの、出力バッファをstd::cerr用のバッファに切り替えます。この、設定は保持されませんので、１回ごとに指定しなければなりません。
#### 戻り値
　SubProcessオブジェクト。
### SubProcess &Raw()noexcept
#### 説明
　次の、入力ストリームオペレーションは、RAWデータとして扱い、その後、Flush相当のオペレーションを自動で行います。通常、`"Hello\n World"`の様なstringの途中に改行文字を入れると、`"Hello\n"`で一旦書き込みをします。`" World"`の部分はバッファに残り、書き込みをしません。Raw()は流し込まれたデータをバイナリデータとして扱い、`"Hello\n World"`をそのまま書き込みます。

:::note info
　データサイズが内部バッファより大きい場合、データの先頭から、内部バッファのサイズまでを、データの塊として切り出します。それを一旦書き込みをして、また、切り出し・書き込みを順次行っていきます。
:::
#### 戻り値
　SubProcessオブジェクト。
# 終わりに
　「**[C++]Pythonに追いつきたい! subprocessの実装**」の解説は以上となります。この記事が皆様の閃きや発想のきっかけになりましたら幸いです。
　また、ご意見、ご感想、ご質問など、お待ちしております。

