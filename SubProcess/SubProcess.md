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
![image.png](https://qiita-image-store.s3.ap-northeast-1.amazonaws.com/0/3813628/bef6dcc1-1103-13ce-1aa3-0747aaa3432a.png)
 
　ソースコードの中にはデバッグ用のライブラリも含んでいます。本質ではない為、今回は説明を割愛いたします。

:::note info
 　この記事で紹介しているソースコードは、公開した時点から変更を加えている事があります。そのため、元の記事とは異なる結果を得る場合があります。また、ソースコードを機能別にディレクトリを分ける等の、改善を行う可能性があります。
:::
 
### デモコード
 　下記にデモコードを記載します。その後、番号のコメントが付けられているところの、解説を順次していきます。
```test2.cpp
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
      if (!(sp.Await(200) >> str) )
         return 1;
      cout << str;

      sp.SetTimeOut(200); // ６

      if (!(sp << "chcp\n"))	// ７
         return 1;
      for (; sp.IsReadable(100);) {
         if( !(sp >> cout) )
            return 1;
      }

      if (!(sp << "chcp"))	// ８
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

      if (!(sp << "chcp "))// ９
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

      if (!(sp << "exit" << endl))	// １０
         return 1;
      if(sp.IsReadable(100)) {
         sp >> cout;
      }

      if (!(sp.WaitForTermination(INFINITE))) // １１
         return 1;

      DWORD dw = sp.GetExitCodeSubProcess(); // １２
      cout << "\nExit code is " << dw << endl;

      sp.Pclose(); // １３

      cout << "\nThe SubProcess demo has successfully concluded." << endl;
   }
   _CrtDumpMemoryLeaks();
}
 ```
#### 1\. sp.SetUseStdErr(true);
　起動する前に、子プロセスからの標準エラーの書き込みを標準エラーとして受け取るかどうかを設定します。`Popen`で起動した後に設定しようとすると、エラーになり、__numErrには`STILL_ACTIVE`がセットされます。

#### 2\. sp.Popen(R"(cmd.exe)")
　子プロセスとしてcmd.exeを起動。
#### 3\. sp.Await(1000) >> cout
　子プロセスからの書き込みを`std::cout`に、出力します。`Await(1000)`は書き込みがあるまで1000ミリ秒待つという意味です。それでも、書き込みが無い場合、`WAIT_TIMEOUT`エラーになり、次のオペレーションが、実行できない場合があります。
#### 4\. 
``` test2.cpp
      if (!(sp >> cout)) { 	// ４
         debug_fnc::ENOut(sp.GetLastError());
         sp.ResetFlag();
      }
```
　子プロセスから書き込みが、無い事が判っている時に、更に読み込もうとしています。当然`WAIT_TIMEOUT`エラーになります。その後、エラー内容を`debug_fnc::ENOut(sp.GetLastError());`で、デバッグ出力しています。その後、エラーステータスを解消する為に`ResetFlag`を呼び出しています。
#### 5\. 
``` test2.cpp
      if (!(sp << "chcp" << endl))	// ５
         return 1;
      if (!(sp.Await(200) >> str) )
         return 1;
      cout << str;
```
　SubProcessオブジェクト**sp**に文字列と、std::endlを入力しています。`sp.Await(200) >> str`で、子プロセスからの書き込みを`std::string`の`str`に、書き込んでいます。

#### 6\. sp.SetTimeOut(200); // ６
　デフォルトタイムアウトの設定を変更しています。初期は100ミリ秒になっています。非同期なので、状況に応じて変更しなくてはならない事があります。

#### 7\. 
``` test2.cpp
      if (!(sp << "chcp\n"))	// ７
         return 1;
      for (; sp.IsReadable(100);) {
         if( !(sp >> cout) )
            return 1;
      }
```
　行末を`std::endl`の代わりに、改行文字を"chcp"の後に加えています。これでも子プロセスに送信されます。

#### 8\.
``` test2.cpp
      if (!(sp << "chcp"))	// ８
         return 1;
      if (sp.IsReadable(100)) {
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
```
　`chcp`の後に、std::endlも、改行も加えなければどうなるかを見ています。`cmd.exe`は改行込みでコマンドを受け取ります。改行無しで送った場合どういう反応になるのでしょうか？
　`IsReadable(100)`は、読み込み可能か、どうか、100ミリ秒の猶予を与えて子プロセスからの反応を待っています。読み込みが出来なくても`WAIT_TIMEOUT`エラーにはなりません。デフォルト値はタイムアウト値と同じです。最初の`IsReadable`では、読み込みが出来ません。`sp.Flush()`はストリームをフラッシュしています。この後の、`sp.IsReadable()`でも読み込みは出来ません。そして、`sp << endl`を送った後、`sp.IsReadable(100)`を実行すると、読み込みが出来ました。想定通りです。

#### 9\.
``` test2.cpp
      if (!(sp << "chcp "))// ９
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
```
　"chcp "だけ書き込んでおいて、`cin`からコートページの入力を待つようにしています。ここで、コードページ（例えば、"932"+エンター）を入力すると、`cin`から反応があります。`sp.CErr().IsReadable()`は標準エラーへの書き込みが無いか、見ています。
##### 試しに無効なコードページを入力すると
　例えば、「**'0'+エンター**」を入力すると、「**無効なコード ページです**」 と表示されます。このまま終了すると`GetExitCodeSubProcess`は、子プロセスの終了コード「**１**」を返します。

```プロンプト.txt
chcp 0

F:\test\Reversi\TestProject>無効なコード ページです
exit

Exit code is 1

The SubProcess demo has successfully concluded.
```


#### 10\.
``` test2.cpp
      if (!(sp << "exit" << endl))	// １０
         return 1;
      if(sp.IsReadable(100)) {
         sp >> cout;
      }
```
　`cmd.exe`を終了させるコマンド`exit`を送信しています。この後、子プロセスから送信があれば表示するようにしています。

#### 11\.
``` test2.cpp
      if (!(sp.WaitForTermination(INFINITE))) // １１
         return 1;
```
　子プロセス`cmd.exe`が、終了する事が分かっているので、終了を待ちます。`cmd.exe`のクリーンナップに時間が、かかる事があるので、これをしておかないと異常終了になる場合があります。std::threadの`join`と役目は似ています。

#### 12\. 
``` test2.cpp
      DWORD dw = sp.GetExitCodeSubProcess(); // １２
      cout << "\nExit code is " << dw << endl;
```
　exitコードの取得をしています。終了していないのに取得しようとしたときには、`STILL_ACTIVE`(259)が返されます。

#### 13\.
``` test2.cpp
      sp.Pclose(); // １３
```
　プロセスハンドルを閉じています。この後、終了コードの取得は出来なくなります。次のプロセスを起動する事が出来ます。


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
　成功したら`true`、それ以外は`false`。詳細なエラーコードは`SubProcess::GetLastError`で取得できます。エラーコードは「winerror.h」で定義されています。
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
　最後のオペレーションが成功したかどうかを返します。オペレーションが失敗していた場合、`SubProcess &operator>>(std::string &str)`、`SubProcess &operator>>(std::ostream &os)` 等の一部のオペレーション以外は、`ResetFlag`が実行されるまで、次のオペレーションは実行されません。
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
　読み込み可能ならば`true`、それ以外は`false`。**直前のオペレーションでエラーが発生しても、バッファーにデータが入っていれば`true`を返します。**
### bool SetUseStdErr(bool is_use)
#### 説明
　子プロセスから標準エラーの書き込みを、標準出力とは別の標準エラーとして、受け取るかどうかを設定します。`Popen`を実行する前にセットする必要があります。
#### 引数
##### bool is_use
　標準エラーを使うなら`true`、それ以外は`false`。
#### 戻り値
　成功すると`true`、それ以外は`false`。もし、起動中に呼び出されると、`false`を返して、`SubProcess::GetLastError()`は`STILL_ACTIVE`を返します。

### SubProcess &operator<<(const std::string &str)
#### 説明
　std::stringを受け付け、それを子プロセスの標準入力に出力します。
#### 引数
##### std::string str
　子プロセスに渡すデータ。テキストデータに限定しません。
##### 戻り値
　SubProcessオブジェクト。この関数を実行して、エラーがあったかどうかは、`explicit operator bool() const noexcept`、`SubProcess::GetLastError()`の他の関数で調べる事が出来ます。

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
　numで指定した時間を最大として、ブロッキングをしてAPCキューの完了を待ちます。指定された時間前にAPCキューのプロシージャが実行された場合、ブロッキングは解除されます。タイムアウトになってもそれはエラーとして記録されません。他の要因でエラーになった場合、内部変数に記録され、次からのオペレーションは失敗します。エラーコードは`GetLastError()`で取得できます。APCキューに入っているプロシージャが必ずしもこのSubProcessオブジェクトとは、限りません。
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
　子プロセスが送信できるかスレッドを渡して確認します。読み込み済みバッファーが空でない場合、内容をstrに書き込みます。バッファーが空の場合、タイムアウト値まで待ちます。それでも子プロセスからの入力が無い場合、タイムアウトになります。`SubProcess::GetLastError`で、`WAIT_TIMEOUT`を返します。
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
　SubProcessオブジェクト。タイムアウトが発生した場合、`SubProcess::GetLastError`で`WAIT_TIMEOUT`を取得できます。
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
### ~~SubProcess &Raw()noexcept~~ 
> この関数はバイナリ送信用に追加されましたが、SubProcess事態をバイトストリームに変更しましたので削除しました。バイナリ転送には、Flushを使用してください。

# 終わりに
　「**[C++]Pythonに追いつきたい! subprocessの実装**」の解説は以上となります。この記事が皆様の閃きや発想のきっかけになりましたら幸いです。
　また、ご意見、ご感想、ご質問など、お待ちしております。

