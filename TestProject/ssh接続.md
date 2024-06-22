# この記事の対象読者
　**Python**の**paramiko**モジュールの使われ方を見たとき、「**えっ、こんな手軽にSSHが使えるんだ！**」と、感じた事はないでしょうか？「C++で、ワンポイントで**手軽にSSHを使いたい**。」そんなニッチな方向けの記事です。ちなみに、C++で**がっつりSSHを使いたい**人向けには、[libssh2](https://libssh2.org/ "libssh2")というライブラリが公開されていますので、こちらをお勧めします。
　また、この技法は、

https://qiita.com/GoldSmith/items/aeec5a42fcc76dd6f37d

の、SubProcessクラスを使って構築しています。こちらの記事も、よろしければご覧ください。
## 仕組みは子プロセスのシェルでSSH接続のスクリプトを実行する
　これだけです。
## 事前準備としてリモートで接続するコンピュータを用意して、鍵認証でログイン出来るようにしておきましょう
　`ssh リモートホスト名`+`Enter`、または、`ssh ユーザー名@リモートホスト名`+`Enter`でログイン出来るようにしておきましょう。
 ```コンソール.画面

Microsoft Windows [Version 10.0.19045.4529]
(c) Microsoft Corporation. All rights reserved.

C:\Users\usera>
 ```
 　このような画面から`ssh リモートホスト名（または、IPアドレス）`を入力すると、
```コンソール.画面
Microsoft Windows [Version 10.0.22631.3737]
(c) Microsoft Corporation. All rights reserved.

usera@R5600 C:\Users\usera>
```
と、リモートホストに、繋がればOKです。


## デモコードとスクリプト
### この記事で使うソースコードへのリンク
[GitHubへのリンクはここです。Visual Studio 2022用に設定されたslnファイルもあります。](https://github.com/NewGoldSmith/SubProcess "https://github.com/NewGoldSmith/SubProcess")
　 **TestProject**をスタートアッププロジェクトに設定し、ソリューションエクスプローラーから**test3.cpp**を選択し、プロパティの設定で**全般->ビルドから除外**項目を**いいえ**に設定し、**test3.cpp以外**は**はい**に設定し、ターゲットCPUをx64に設定し、`F5`を押下すると実行できます。
![image.png](https://qiita-image-store.s3.ap-northeast-1.amazonaws.com/0/3813628/63a9cc5c-1a31-ca1e-5a95-5dd6e76dfac8.png)
  
　ソースコードの中にはデバッグ用のライブラリも含んでいます。本質ではない為、今回は説明を割愛いたします。
 
:::note info
 　この記事で紹介しているソースコードは、公開した時点から変更を加えている事があります。そのため、元の記事とは異なる結果を得る場合があります。また、ソースコードを機能別にディレクトリを分ける等の、改善を行う可能性があります。
:::
## デモコード
　下記にデモコードを記載します。その後、番号のコメントが付けられているところの、解説を順次していきます。

```test3.cpp
// test3.cpp
#include "test2.h"
using namespace std;

int main() {
	{
		string str;										// １
		SubProcess sp;
		if (!sp.Popen(R"(cmd.exe /c RemoteCmd.bat)"))
			return 1;

		sp.SleepEx(2000);								// ２
		
		for (; sp.IsActive();) {					// ３
			if (sp.IsReadable()) {
				if (!(sp >> cout))
					return 1;
				else
					sp.SleepEx(10);
					continue;

			} else {
				if (!(sp << cin))
					return 1;
				sp.SleepEx(10);
			}
		}

		if (!(sp.WaitForTermination(INFINITE)))// ４
			return 1;

		DWORD dw = sp.GetExitCodeSubProcess();	// ５
		cout << "\nExit code is " << dw << endl;

		sp.Pclose();									// ６
	}
	_CrtDumpMemoryLeaks();

}
```
### １、**cmd.exe**を子プロセスとして起動
```test3.cpp
		string str;										// １
		SubProcess sp;
		if (!sp.Popen(R"(cmd.exe /c RemoteCmd.bat)"))
			return 1;
```
　子プロセスとして`cmd.exe`を起動と共に、バッチファイルの`RemoteCmd.bat`を実行。バッチファイルの中身は下記のようになっています。
```RemoteCmd.bat
@ssh 192.168.100.205 "cmd.exe" 
```
　リモートコンピュータで`cmd.exe`を起動するようにしています。
### ２、`sp.SleepEx(2000)`で、リモートホストの接続完了待ち
```test3.cpp
		sp.SleepEx(2000);								// ２
```
　`sp.SleepEx(2000)`で最大２秒待ちます。それまでに、リモートコンピュータからデータが送られてきたら、それで待ちは終了です。
　接続に成功すると、例えば次のように表示されます。
```ssh接続済み.コンソール
Microsoft Windows [Version 10.0.22631.3737]
(c) Microsoft Corporation. All rights reserved.

usera@R5600 C:\Users\usera>
```
### ３、送信と受信を繰り返す
```test3.cpp
		for (; sp.IsActive();) {					// ３
			if (sp.IsReadable()) {
				if (!(sp >> cout))
					return 1;
				else
					sp.SleepEx(10);
					continue;

			} else {
				if (!(sp << cin))
					return 1;
				sp.SleepEx(10);
			}
		}
```
　ここで、送信と受信を繰り返します。`chcp`コマンドを入力してみましょう。
```ssh接続済み.コンソール
usera@R5600 C:\Users\usera>chcp
chcp
݂̃R[h y[W: 932

usera@R5600 C:\Users\usera>
```
　文字化けしていますね･･･。では次に、`exit`と入力してみましょう。ssh接続が切断されます。
### ４、切断された後、ローカルのcmd.exeが終了するのを待つ
```text3.cpp
		if (!(sp.WaitForTermination(INFINITE)))// ４
			return 1;
```
　子プロセスが終了する前に、親プロセスが終了してしまわないようにします。このプロセスを踏んでおかないと、異常終了となる可能性があります。
　このオペレーションが成功すると、
```ssh接続済み.コンソール
usera@R5600 C:\Users\usera>exit
exit
```
といった表示が出ます。
### ５、exitコードを取得する
```test3.cpp
		DWORD dw = sp.GetExitCodeSubProcess();	// ５
		cout << "\nExit code is " << dw << endl;
```
　exitコードを取得し、表示します。
```ssh切断後.コンソール
Exit code is 0
```
　と、表示されます。
### ６、子プロセス操作ハンドルを閉じる
```test3.cpp
		sp.Pclose();									// ６
```
　これをした後は、exitコードは取得できなくなります。この後、SubProcessクラスは別の子プロセスを立ち上げる事が出来ます。


## 以上で、デモコードの説明終了です
　いかがでしたでしょうか。
# 終わりに
　「**[C++]Pythonに追いつきたい！２ 自作プログラムでSSH接続する**」の解説は以上となります。この記事が皆様の閃きや発想のきっかけになりましたら幸いです。
　また、ご意見、ご感想、ご質問など、お待ちしております。

