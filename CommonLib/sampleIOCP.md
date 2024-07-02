# この記事の対象読者
## 次の項目に当てはまる人向きです。
 ●Windowsの**IOCP**とは何なのか知りたい。
 ●Microsoft公式ドキュメントの**IOCP**のサンプルコードは、**あっちこっちに飛んで何をしているのかよくわからない。**

https://learn.microsoft.com/ja-jp/windows/win32/ipc/named-pipe-server-using-completion-routines

　そんな方向けの記事です。
　また、この技法は、

https://qiita.com/GoldSmith/items/1de197c10c05c054461a

の、**メモリープールコンテナ**クラスを使って構築しています。こちらの記事も、よろしければご覧ください。
# きっかけ
　[[C++]Pythonに追いつきたい! subprocessの実装](https://qiita.com/GoldSmith/items/aeec5a42fcc76dd6f37d "[C++]Pythonに追いつきたい! subprocessの実装")の記事を書いた後、ずっと気になっていたことがありました。「ここでの**WAIT_IO_COMPLETION**の比較は何か違うよな。」とか、「パイプタイプ**PIPE_TYPE_MESSAGE**はこれで**いいのか**？**でも**、[Microsoftサンプルプログラム](https://learn.microsoft.com/ja-jp/windows/win32/ipc/named-pipe-server-using-completion-routines "Microsoftサンプルプログラム")に、**こう書いてあった**な。」とか、「**パイプバッファーは必要なの？**」など疑問が残っていました。それで、改めて**私自身が、IOCPの処理の流れが理解でき**、パイプ設定を変えるとどうなるか判るプログラムを書いてみる事にしました。
# なぜ非同期（IOCP）が必要なのか？

　**IOCP**(Input/Output Completion Port)とは、Windowsの非同期データ転送処理をする為の一連のシステムサービスです。一般に、CPUの計算速度に比べて、周辺機器IOはのんびりとしたデータ転送をします。その為、IOのデータ転送にCPUがかかりっきりになると、CPUが計算できたはずの機会を、無駄に過ごすことになります。そこで、CPUは計算しながら、時々IOに気にかけるような、仕組みが用意されました。それが**IOCP**です。
## 非同期処理は、メモリーを期限未定で貸して処理をしてもらうもの
　`ReadFileEx`で例えると、システムに「**読み込んだデータをこれに書き込んでおいて。**」とメモリ領域を渡して、暫くして「**あれ、どうなった？**」と、システムに尋ねて、その時、システムが読み込みを完了していたら「**ああ、あれね。**」といって完了通知コールバック関数を呼び出し、尋ねた相手に「**完了しました。**」と返事を返す仕組みです。ですので、**至急これを読み込んでデータがほしい！**　みたいな場合は、非同期の良さは出ません。
## 通常、この仕組みを使う事はあるの？
　httpsでインターネットから非同期でデータを読み込むなどの、ニーズが考えられますが、https接続は、すでにライブラリとして[libcurl](https://curl.se/libcurl/ "libcurl")や、[httplib.h](https://github.com/yhirose/cpp-httplib/ "httplib.h")があるので、別のスレッドを立ててこのライブラリを使い、必要になったタイミングでデータを取得するというのが、一般的な方法になると思いますので、下層のIOCPを使って何かやるという事はありません。
## 使うとすれば、デファクトスタンダード規格になってないカスタムなデディケーテッドサーバーとか
　例えば、実在するボードゲームをコンピュータ化した物を、思考エンジン同士で競い合わせるデディケーテッドサーバーを作る目的で使うのはありかと思います。スレッドプールと組み合わせれば、通信があるときのみコンピュータに負荷を持たせる事も出来、効率のいい物が出来そうです。ネットワークの最大接続数も、私個人が確認したところ、1nicあたり16000強はいけます。ただ、高性能のルーターでないと最大セッション数に縛られるかもしれません。
　通常、**思考エンジンは**ボードの子プロセスとして起動され、**ボードとのプロトコルは備えていますが、エンジン同士でネットワークを通じて競うプロトコルは持っていません。** そこを補うプログラムと、プロトコルを作ればエンジン同士の対戦が出来るわけです。
## この記事は取り合えずICPOの流れをつかむことに特化しています
　「プログラミングWindow」では、この仕組みは説明されていません。なので、理解する為の情報源は[Microsoft公式サイト](https://learn.microsoft.com/ja-jp/windows/win32/ipc/named-pipe-server-using-completion-routines "完了ルーチンを使用した名前付きパイプ サーバー")になるわけですが、このIOCPを使ったサンプルプログラムは、判りやすいとは私には言えません。プログラムの流れが非常に判りにくくなっています。また、パラメーターをいろいろ変えた時の、動作の確認をするのもめんどくさそうです。それで、今回、ICPOの流れを視覚化して見るプログラムを作ってみました。

### この記事で取り扱うプログラムの事前説明
#### 名前付きパイプサーバー側とクライアント側に分かれ、それぞれが別スレッドで動き、データの流れ、タイミングをみます
　Microsoftのサンプルプログラムであれば、パイプサーバー(クライアントの接続を待つ側）と、クライアント（サーバーのパイプに接続する側）が別々のプロセスで動くように作られています。しかし、この記事で使う、`sampleIOCP`は一つにまとめました。
#### サーバー側はパイプにデータを書き込み、クライアント側からの返事を待ちます
　書き込み用、読み込み用２つの名前付きパイプを使っています。
#### クライアント側はサーバー側からきたデータをそのまんまエコーします
　読み込み、書き込みが一つのループに収まっています。
#### パイプはバイトストリームとして扱うように設定
　名前付きパイプは、データを塊として扱う`PIPE_TYPE_MESSAGE`、サイズが確定できない、バイトが川の流れのように次々転送される`PIPE_TYPE_BYTE`モードか、どちらかを選択できます。このプログラムでは`PIPE_TYPE_BYTE`モードを指定しました。
## デモコードと解説
### この記事で使うソースコードへのリンク
[GitHubへのリンクはここです。Visual Studio 2022用に設定されたslnファイルもあります。](https://github.com/NewGoldSmith/SubProcess "https://github.com/NewGoldSmith/SubProcess")
　 **TestProject**をスタートアッププロジェクトに設定し、ソリューションエクスプローラーから**testIOCP.cpp**を選択し、プロパティの設定で**全般->ビルドから除外**項目を**いいえ**に設定し、**testIOCP.cpp以外**は**はい**に設定し、ターゲットCPUをx64に設定し、`F5`を押下すると実行できます。
![sampleIOCP.png](https://qiita-image-store.s3.ap-northeast-1.amazonaws.com/0/3813628/ca831fa0-33ee-edec-2779-5a8dc9c9eb1f.png)
　ソースコードの中にはデバッグ用のライブラリ、表示と表示の時間のギャップを測るライブラリも含んでいます。本質ではない為、今回は説明を割愛いたします。
 
:::note info
 　この記事で紹介しているソースコードは、公開した時点から変更を加えている事があります。そのため、元の記事とは異なる結果を得る場合があります。また、ソースコードを機能別にディレクトリを分ける等の、改善を行う可能性があります。
:::

## コードの解説
　下記にデモコードを記載します。次に実行結果の画面を記載します。その後、番号のコメントが付けられているところの、解説を順次行います。
### mainのコード
```testIOCP.cpp
#include "testIOCP.h"
using namespace std;
int main() {
	{
		sampleIOCP s;// 1
		if (!(s << "Hello, World!"))// 2
			return 1;
		string str;
		if (!(s >> str))// 3
			return 1;
		s.OrCout.Push("Main:\"" + str + "\"");// 4
	}
	_CrtDumpMemoryLeaks();
	return 0;
}
```
### 実行結果
```コンソール.
Sev:called __ReadFromCli                        0.000 msec //  5
Sev:called __WriteToCli "Hello, World!"         0.010 msec //  6
Sev:WriteToCliCompleted. "Hello, World!"        0.479 msec //  7
Sev:Call SleepEx                                0.448 msec //  8
Cli:Client successfully read."Hello, World!"    0.704 msec //  9
Cli:Client successfully wrote."Hello, World!"   1.096 msec // 10
Sev:ReadFromCliCompleted."Hello, World!"        1.046 msec // 11
Sev:called __ReadFromCli                        1.318 msec // 12
Sev:called SleepEx "WAIT_IO_COMPLETION"         1.052 msec // 13
Main:"Hello, World!"                            0.831 msec // 14
```
### mainソースコードの解説

#### １、sampleIOCP s;
##### `sampleIOCP`オブジェクトを作成
　デフォルトコンストラクタを使います。
#### ２、s << "Hello, World!"
##### パイプサーバー側にデータを送る
　sampleIOCPオブジェクトが文字列を読み込み、名前付きパイプを通じてクライアント側へ書き込みをします。その後、内部では、クライアントが読み込み、クライアントからサーバーに書き込みをします。
#### ３、s >> str
##### クライアントから返ってきた文字列をstd::stringに書き込む
#### ４、`s.OrCout.Push("Main:\"" + str + "\"");`
##### 結果を表示する
　`s.OrCout.Push("Main:\"" + str + "\"");`の、`OrCout`は、シリアライズ化して順番を維持して表示する、OrderedCOutというクラスのオブジェクトです。前の表示との時間差を表示する機能もあります。

:::note info
　このOrderedCOutクラスの解説は、今回は割愛します。
:::

### 実行結果の表示の解説
#### ５、Sev:called __ReadFromCli                        0.000 msec
##### サーバー側、`__ReadFromCli`呼び出し
　表示ですが、行頭から、**Sev:** はサーバー側のメッセージである事を現しています。クライアント側のメッセージは**Cli:** から始まります。「**`Sev:called __ReadFromCli`** 」は、サーバー側が 「**`__ReadfromCli`**　」という関数の呼び出しをした事を表示しています。最後の「**msec**」は前の表示との時間差です。この表示は一番最初の表示なので、0.000 msecとなっています。
　「何かデータが来たら読み込んでおいて。」という指示です。
#### ６、Sev:called __WriteToCli "Hello, World!"         0.010 msec
##### サーバー側、書き込みを行う
　「**`__WriteToCli`** 」関数が呼ばれ、**`"Hello, World!"`** のというデータの書き込みをしたことを表しています。前のメッセージの表示から**0.010 msec**時間が経っています。
　この表示される時間は、時間差を計算するプログラムの時間や、その他の作業時間も含まれています。
#### ７、Sev:WriteToCliCompleted. "Hello, World!"        0.479 msec
##### サーバー側、書き込み完了コールバック関数が呼ばれた
　このコールバックで、書き込みをする為に、システムに貸し出していたメモリーを回収します。このプログラムの場合、メモリープールにメモリを返却します。
#### ８、Sev:Call SleepEx                                0.448 msec
##### サーバー側、`SleepEx`呼び出し
　コールバック関数にスレッドが回る様にします。これのおかげでWriteToCliCompletedというスレッドが回り（CPUのプログラムカウンタにコールバックのアドレスがロードされ、実行され）ました。
#### ９、Cli:Client successfully read."Hello, World!"    0.704 msec
##### クライアント側がパイプから"Hello, World"を読み込みの成功
#### １０、Cli:Client successfully wrote."Hello, World!"   1.096 msec
##### クライアント側、書き込みが成功
　**同期書き込み**しています。この後ループで**読み込み待ち**になります。
#### １１、Sev:ReadFromCliCompleted."Hello, World!"        1.046 msec
##### サーバ―側、読み込み完了コールバック関数が呼ばれた
　「**５、**」でコールしていた`__ReadFromCli`の完了報告がコールバックにてされた事を表しています。
#### １２、Sev:called __ReadFromCli                        1.318 msec
##### サーバー側、`__ReadFromCli`を呼び出した
　次のデータを受け取る為に、`__ReadFromCli`が呼ばれました。もちろん次のデータが来なくて、プログラムが終了する事もあります。
#### １３、Sev:called SleepEx "WAIT_IO_COMPLETION"         1.052 msec
##### サーバー側、呼んでいた`SleepEx`の戻り値が返され、その結果が`WAIT_IO_COMPLETION`（IO操作が完了した旨の数字）であった
　「**８、**」の呼び出しの戻り値。
#### １４、Main:"Hello, World!"                            0.831 msec
##### main関数（testIOCP.cppの「**４、**」）のコードを実行
　サーバーがクライアントから送られてきたデータを取得し、それをメイン関数の`std::string`に渡し、main関数内の`s.OrCout.Push("Main:\"" + str + "\"");`で表示しています。
### 以上、デモコードの解説でした
## ここでちょっと実験
### ストリームってサイズ不定だよね。バッファのサイズ大丈夫？
　ストリームタイプは一般的なWebサイトにアクセスする時にも使われます。ネットはソケットですが、Windowsシステムにしてみれば、同じような物です。この記事の内容は、ソケットでも同じような仕組みが使えます。
　`ReadFileEx`や`WrieFileEx`に渡すバッファサイズが小さい場合どのようになるか見ていきましょう。次のコードは**OVERLAPPED_CUSTOM**という、OVERLAPPED構造体に、**`this`ポインター**とchar型の **`buffer`** を加えたものを定義しています。
```sampleOverLapped.h
class  sampleIOCP {
    // 他のコードもありますが省略
	struct OVERLAPPED_CUSTOM {
		OVERLAPPED ol{};
		sampleIOCP* self{};
		char buffer[BUFFER_SIZE_OL]{};
	};
 }
```
　この**OVERLAPPED_CUSTOM::buffer**を使って、読み込んだり、書き込んだりしているのですが、このサイズより大きいデータはどう処理されるのか、見てみましょう。
### ソースを一部書き換える
#### testIOCP.cpp
```testIOCP.cpp
#include "testIOCP.h"
using namespace std;
int main() {
	{
		sampleIOCP s;// 1
		if (!(s << "Hello, World!"))// 2
			return 1;
		Sleep(1000); // *5
		string str;
		if (!(s >> str))// 3
			return 1;
		s.OrCout.Push("Main:\"" + str + "\"");// 4
	}
	_CrtDumpMemoryLeaks();
	return 0;
}
```
　「*5」の所、`Sleep(1000);`を加えます。
#### ../CommonLib/sampleIOCP.h
```../CommonLib/sampleIOCP.h
class  sampleIOCP {
	static constexpr DWORD BUFFER_SIZE_OL = 0x4;
	static constexpr DWORD BUFFER_SIZE_CL_SIDE = 0x400;
	static constexpr DWORD BUFER_SIZE_PIPE = 0x100;
```
　`BUFFER_SIZE_OL = 0x10`の所を、`BUFFER_SIZE_OL = 0x4`に変更します。
### 実行してみる
```コンソール.
Sev:called __ReadFromCli                        0.000 msec
Sev:called __WriteToCli "Hell"                  0.010 msec
Sev:WriteToCliCompleted. "Hell"                 0.781 msec
Cli:Client successfully read."Hell"             0.430 msec
Sev:called __WriteToCli "o, W"                  0.427 msec
Sev:WriteToCliCompleted. "o, W"                 0.427 msec
Sev:called __WriteToCli "orld"                  0.638 msec
Sev:WriteToCliCompleted. "orld"                 0.432 msec
Sev:called __WriteToCli "!"                     0.553 msec
Sev:WriteToCliCompleted. "!"                    0.573 msec
Cli:Client successfully wrote."Hell"            0.553 msec
Cli:Client successfully read."o, World!"        0.457 msec
Cli:Client successfully wrote."o, World!"       1.970 msec
Sev:Call SleepEx                                1004.541 msec
Sev:ReadFromCliCompleted."Hell"                 1.321 msec
Sev:called __ReadFromCli                        1.127 msec
Sev:ReadFromCliCompleted."o, W"                 0.699 msec
Sev:called __ReadFromCli                        0.578 msec
Sev:ReadFromCliCompleted."orld"                 0.492 msec
Sev:called __ReadFromCli                        0.612 msec
Sev:ReadFromCliCompleted."!"                    0.594 msec
Sev:called __ReadFromCli                        0.571 msec
Sev:called SleepEx "WAIT_IO_COMPLETION"         0.566 msec
Main:"Hello, World!"                            0.580 msec
```
### 見て判る事
**１：** パイプサーバー側は**細かく分けて書き込んでいる**のが判ります。サーバー側のプログラムで細かく分けるようにしています。**このコードは自分で書かなければなりません**。クライアント側は、その分けた文字列を**そのまま受け取る事もあり**、また、**まとめて受け取る事もある**事が判ります。クライアント側のバッファサイズは1024バイトで十分用意しています。
**２：** クライアント側は２回に分けて、読み込み、また、書き込んでいるのが判ります。
**３：** `Sev:Call SleepEx`が表示されるまで、**１秒強**経過しているのが判ります。これはmain.cppの`Sleep(1000);`でメインスレッドがスリープしているのが、原因と考えられます。この、`Sleep(1000);`は、サブスレッドでクライアント側が、**エコー出来る余裕**を与えるつもりで加えました。
**４：** サーバー側の読み込みも細かく読み込んでいるのが判ります。
### 更に書き換えてみる
#### ../CommonLib/sampleIOCP.h
```../CommonLib/sampleIOCP.h
class  sampleIOCP {
	static constexpr DWORD BUFFER_SIZE_OL = 0x4;
	static constexpr DWORD BUFFER_SIZE_CL_SIDE = 0x400;
	static constexpr DWORD BUFER_SIZE_PIPE = 0x00;
```
　**`BUFER_SIZE_PIPE`** を0にしてみます。これは、パイプを作った時に指定するバッファーサイズです。
```../CommonLib/sampleIOCP.cpp
::CreateNamedPipeW(
			wstr.c_str()
			, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED
			, PIPE_TYPE_BYTE | PIPE_WAIT | PIPE_ACCEPT_REMOTE_CLIENTS
			, PIPE_UNLIMITED_INSTANCES
			, BUFER_SIZE_PIPE
			, BUFER_SIZE_PIPE
			, 0
			, NULL)
```
　このように、サーバー側パイプを作るときに、指定しています。
### 実行してみる
```コンソール.
Sev:called __ReadFromCli                        0.000 msec
Sev:called __WriteToCli "Hell"                  0.010 msec
Sev:called __WriteToCli "o, W"                  0.447 msec
Sev:called __WriteToCli "orld"                  0.459 msec
Cli:Client successfully read."Hello, World"     0.647 msec
Sev:WriteToCliCompleted. "Hell"                 0.436 msec
Sev:WriteToCliCompleted. "o, W"                 0.515 msec
Sev:WriteToCliCompleted. "orld"                 0.526 msec
Sev:called __WriteToCli "!"                     0.476 msec
Sev:Call SleepEx                                997.969 msec
Sev:ReadFromCliCompleted."Hell"                 1.739 msec
Sev:called __ReadFromCli                        0.622 msec
Sev:ReadFromCliCompleted."o, W"                 0.525 msec
Sev:called __ReadFromCli                        0.565 msec
Sev:ReadFromCliCompleted."orld"                 0.967 msec
Cli:Client successfully wrote."Hello, World"    0.631 msec
Cli:Client successfully read."!"                0.564 msec
Sev:called __ReadFromCli                        0.639 msec
Sev:WriteToCliCompleted. "!"                    0.631 msec
Sev:called SleepEx "WAIT_IO_COMPLETION"         0.611 msec
Main:"Hello, World"                             0.596 msec
```
　最後の行、**Main:"Hello, World"** となっています。main関数で表示する時に、最後の文字 **「!」が間に合っていません**。**何回か実行すると間に合う時もあるようです。** また、別の時には `Main:"Hello, W"`  と、**全然間に合っていないときもあり**ました。このことから、**パイプのバッファを設定する意味はありそうです。**
## 実験の結果、判った事
　この記事を書きながら、実験を行ったところもあり、その中で感じた事です。
**１、**　受信の時は、バッファーサイズに合ったデータが格納されていく。
**２、**　**パイプバッファー**の設定も**受信特性**が変化する。
**３、**　ストリームを扱う時は、**上層プロトコルでデータの区切りを判断する**必要がありそうだ。ソケット通信でも当てはまる事だと思われる。
## 以上で実験終わり
### まとめ
#### １、IOCPの知識が曖昧だったので、IOCPの流れが判るプログラムを作った。
#### ２、パイプのタイプは**PIPE_TYPE_BYTE**を選択。
#### ３、計測したところ、条件によっては、データ取得タイミング時にデータが、全て揃ってない事があった。**PIPE_TYPE_MESSAGE**であれば、データが来たか来てないかの二択であったと思われる。**PIPE_TYPE_BYTE**は身近な用途ではネットワークに使われている。用途に応じて使い分けが必要だと感じた。**PIPE_TYPE_BYTE**か、**PIPE_TYPE_MESSAGE**の選択は悩ましいが、子プログラムがcmd.exeなら改行が文の終わりとなっているので、**PIPE_TYPE_BYTE**が、向いているのではないかと思われる。
#### ４、以前作った**SubProcess**のプログラムは、やはり**IOCP**の使い方が洗練されて無かった。できれば修正するのが、望ましい。
# 終わりに
　「**[C++][Windows]IOCPを理解する**」の解説は以上となります。この記事が皆様の閃きや発想のきっかけになりましたら幸いです。
　また、ご意見、ご感想、ご質問など、お待ちしております。
