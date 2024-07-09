# この記事の対象読者
## 次の項目に当てはまる人向きです。
 ●Windowsの**IOCP**とは何なのか知りたい。
 ●Microsoft公式ドキュメントの**IOCP**のサンプルコードは、**あっちこっちに飛んで何をしているのかよくわからない。**

https://learn.microsoft.com/ja-jp/windows/win32/ipc/named-pipe-server-using-completion-routines

　そんな方向けの記事です。
　また、この技法は、
https://zenn.dev/goldsmith/articles/988bbbdbb3e9ce
の、**メモリープールコンテナ**クラスを使って構築しています。こちらの記事も、よろしければご覧ください。
# きっかけ
　[[C++]Pythonに追いつきたい! subprocessの実装](https://qiita.com/GoldSmith/items/aeec5a42fcc76dd6f37d "[C++]Pythonに追いつきたい! subprocessの実装")の記事を書いた後、ずっと気になっていたことがありました。「ここでの**WAIT_IO_COMPLETION**の比較は何か違うよな。」とか、「パイプタイプ**PIPE_TYPE_MESSAGE**はこれで**いいのか**？**でも**、[Microsoftサンプルプログラム](https://learn.microsoft.com/ja-jp/windows/win32/ipc/named-pipe-server-using-completion-routines "Microsoftサンプルプログラム")に、**こう書いてあった**な。」とか、「**パイプバッファーは必要なの？**」など疑問が残っていました。それで、改めて**私自身が、IOCPの処理の流れが理解でき**、パイプ設定を変えるとどうなるか判るプログラムを書いてみる事にしました。
# 改訂にあたって
　初版を書いて公開したところ、これを読んでくれた方から指摘を受けました。
>･･･非同期IOの完了通知をCreateIOCompletionPort/GetQueuedCompletionStatusで受け取るAPIを指すのが普通です。

 「ああっ、そういうことか。」私は、完了ルーチンを書けば「IOCP」だと思っていたのですが、よく見ると、「ポート」要素が見つかりません。
 　結論を書くと、**APCキュー**と、**I/O 完了ポートキュー**の２種類あり、**SleepEx、WaitForSingleObjectEx**等の**Ex系**は、**APCキュー**を使い、**CreateIOCompletionPort**等でハンドルと結びつけるのが**I/O 完了ポートキュー**を使う様です。
　と、いう事で記事を書き直すことを決め、今に至っています。ですので、記事の内容は以前と違ったものになります。また、私は、IOCPのプログラムを書いたことは無かったのですが、以前、Microsoftのファイルを扱うサンプルコードに、なんかループがあったなと、心の隅に引っかかっていまして、そのループの意味を探りながらコードを書き直しました。そのサンプルコードはどこで見たのかは、思い出せていません。OVERLAPPED構造体をキャストしていた、SleepExを使っていた、というところも思い出したのですが、その時はその意味を理解できていませんでした。
　で、書き直しに取っ掛かり始めたところ、流れは、非同期と同じことが判りました。完了コールバック関数もそのまま利用できます。そこでメンバー関数名はそのままにして、その関数の中のコードを
 ```
 bool sampleIOCP::__WriteToCli(const std::string& str) {
 ･･･
 #ifdef USING_IOCP
 ･･･
 #else
 ･･･
 #endif
 ･･･
}
 ```
といった感じで分ける事にしました。これで、ただの非同期とIOCPとの違いを確かめる事が出来るようになりました。
　また、IOCPといえばサーバーで、マルチスレッドで扱うイメージですが、１クライアントに１ステートを割り当て、交信することが想定されていると思いますで、OVERLAPPED構造体を内包した構造体に、クライアント情報を載せて、処理を進める事になろうかと思います。しかし、このサンプルコードは１クライアントとの交信に絞って流れを見ていきたいので、サーバーはメインスレッド、クライアントはサブスレッドという単純な構成にしています。
　また、記事中の実行結果などは、この記事を書きながら収集していきますので、以前とは違ったものになり、結果も違ったものになると想像されます。

## Bing Copilotに改めてIOCPについて尋ねると、まさかの答えが･･･

>･･･Completion Port（完了ポート）は、Windowsの非同期データ転送処理を行うための一連のシステムサービスで、特にI/O Completion Port (IOCP)として知られています。
IOCPは、CPUの計算速度に比べて、周辺機器IOがのんびりとしたデータ転送を行うための仕組みです。そのため、IOのデータ転送にCPUがかかりっきりになると、CPUが計算できたはずの機会を無駄に過ごすことになります。そこで、CPUは計算しながら、時々IOに気にかけるような仕組みが用意されました。それがIOCPです。･･･

　それ、私が書いた **"この"** 記事なんですけど･･･。
　Qiitaの利用規約通りなので文句は無いのですが、Bing Copilotの情報収集の速さに驚いたとともに、やはり、収集できる情報源が少なく、採用されたって感じなんでしょうか。それだけ、ニッチな題材なんでしょうね。Webで公式ドキュメントに書いてあるのにも関わらず、ニッチさにおいては１、２位を争うのではないでしょうか？
　また内容において、APCキューを使えば、「時々IOに気にかけるような」という表現はおかしいのではないかと思われるかもしれませんが、まあ、これ単体での話であって、それ言いだしたらスレッドプールまで作らなくてはならなくなり、IOCP単体での話でなくなるので、あまり気にしないでいただきたいと思います。
 
# なぜ非同期（IOCP）が必要なのか？

　**IOCP**(Input/Output Completion Port)とは、Windowsの非同期データ転送処理をする為の一連のシステムサービスです。一般に、CPUの計算速度に比べて、周辺機器IOはのんびりとしたデータ転送をします。その為、IOのデータ転送にCPUがかかりっきりになると、CPUが計算できたはずの機会を、無駄に過ごすことになります。そこで、CPUは計算しながら、時々IOに気にかけるような、仕組みが用意されました。それが**IOCP**です。

## 非同期処理は、メモリーを期限未定で貸して処理をしてもらうもの
　`ReadFileEx`で例えると、システムに「**読み込んだデータをこれに書き込んでおいて。**」とメモリ領域を渡して、暫くして「**あれ、どうなった？**」と、システムに尋ねて、その時、システムが読み込みを完了していたら「**ああ、あれね。**」といって完了通知コールバック関数を呼び出し、尋ねた相手に「**完了しました。**」と返事を返す仕組みです。ですので、**至急これを読み込んでデータがほしい！**　みたいな場合は、非同期の良さは出ません。

## 通常、この仕組みを使う事はあるの？
　**クライアント・サーバモデル**なら、SIerさんの領域ですので、まずはこの目的で書く事は無いでしょう。
　別の目的で、httpsでインターネットから非同期でデータを読み込むなどの、ニーズが考えられますが、https接続は、すでにライブラリとして[libcurl](https://curl.se/libcurl/ "libcurl")や、[httplib.h](https://github.com/yhirose/cpp-httplib/ "httplib.h")があるので、別のスレッドを立ててこのライブラリを使い、必要になったタイミングでデータを取得するというのが、一般的な方法になると思いますので、下層のIOCPを使って何かやるという事はないでしょう。

## 使うとすれば、デファクトスタンダード規格になってないカスタムなデディケーテッドサーバーとか
　例えば、実在するボードゲームをコンピュータ化した物を、思考エンジン同士で競い合わせるデディケーテッドサーバーを作る目的で使うのはありかと思います。スレッドプールと組み合わせれば、通信があるときのみコンピュータに負荷を持たせる事も出来、効率のいい物が出来そうです。ネットワークの最大接続数も、私個人が確認したところ、1nicあたり16000強はいけます。ただ、高性能のルーターでないと最大セッション数に縛られるかもしれません。
　通常、**思考エンジンは**ボードの子プロセスとして起動され、**ボードとのプロトコルは備えていますが、エンジン同士でネットワークを通じて競うプロトコルは持っていません。** そこを補うプログラムと、プロトコルを作ればエンジン同士の対戦が出来るわけです。

## この記事は取り合えずICPOの流れをつかむことに特化しています
　「プログラミングWindow」では、この仕組みは説明されていません。なので、理解する為の情報源は[完了ルーチンを使用した名前付きパイプ サーバー](https://learn.microsoft.com/ja-jp/windows/win32/ipc/named-pipe-server-using-completion-routines "完了ルーチンを使用した名前付きパイプ サーバー")と、[`CreateIOCompletionPort`](https://learn.microsoft.com/ja-jp/windows/win32/api/ioapiset/nf-ioapiset-createiocompletionport "CreateIOCompletionPort") /[`GetQueuedCompletionStatus`](https://learn.microsoft.com/ja-jp/windows/win32/api/ioapiset/nf-ioapiset-getqueuedcompletionstatus "GetQueuedCompletionStatus")のドキュメントを元に、カスタマイズしていくわけですが、私は、Microsoftのサンプルプログラムは、判りやすいとは言えないと思います。プログラムの流れが非常に判りにくくなっています。また、パラメーターをいろいろ変えた時の、動作の確認をするのもめんどくさそうです。それで、今回、ICPOの流れを視覚化して見るプログラムを作ってみました。

### この記事で取り扱うプログラムの事前説明

**1. 名前付きパイプサーバー側とクライアント側に分かれ、それぞれが別スレッドで動き、データの流れ、タイミングをみます**
　Microsoftのサンプルプログラムであれば、パイプサーバー(クライアントの接続を待つ側）と、クライアント（サーバーのパイプに接続する側）が別々のプロセスで動くように作られています。しかし、この記事で使う、`sampleIOCP`は一つにまとめました。

**2. 上り用、下り用の２つの名前付きパイプを使います**

**3. クライアント側はサーバー側からきたデータをそのまんまエコーします**
　読み込み、書き込みが一つのループに収まっています。

**4. パイプはバイトストリームとして扱うように設定**
　名前付きパイプは、データを塊として扱う**PIPE_TYPE_MESSAGE**、サイズが確定できない、バイトが川の流れのように次々転送される**PIPE_TYPE_BYTE**モードか、どちらかを選択します。このプログラムでは**PIPE_TYPE_BYTE**モードを指定しました。

## デモコードと解説
### この記事で使うソースコードへのリンク
[GitHubへのリンクはここです。Visual Studio 2022用に設定されたslnファイルもあります。](https://github.com/NewGoldSmith/SubProcess "https://github.com/NewGoldSmith/SubProcess")
　 **TestProject**をスタートアッププロジェクトに設定し、ソリューションエクスプローラーから**testIOCP.cpp**を選択し、プロパティの設定で**全般->ビルドから除外**項目を**いいえ**に設定し、**testIOCP.cpp以外**は**はい**に設定し、ターゲットCPUをx64に設定し、`F5`を押下すると実行できます。
![](https://storage.googleapis.com/zenn-user-upload/69e4bebc8b4a-20240706.png)

　ソースコードの中にはデバッグ用のライブラリ、表示と表示の時間のギャップを測るライブラリも含んでいます。本質ではない為、今回は説明を割愛いたします。
 
> 　この記事で紹介しているソースコードは、公開した時点から変更を加えている事があります。そのため、元の記事とは異なる結果を得る場合があります。また、ソースコードを機能別にディレクトリを分ける等の、改善を行う可能性があります。


## コードの解説
　下記にデモコードを記載します。次に実行結果の画面を記載します。その後、番号のコメントが付けられているところの、解説を順次行います。
### mainのコード
**testIOCP.cpp**
```testIOCP.cpp
#include "testIOCP.h"
#include <conio.h>
using namespace std;
int main() {
	{
		sampleIOCP s;// 1
		if (!(s << "Hello, World!"))// 2
			return 1;
		string str;
		if (!(s.Await(100) >> str))// 3
			return 1;
		s.OrCout.Push("Main:\"" + str + "\"");// 4

		s.OrCout.StopTimer();// 5
		s.OrCout.ShowTimeDisplay(false);// 6
		stringstream ss;// 7
		ss << std::fixed << std::setprecision(3) << s.OrCout.TotalTime() << " msec";
		s.OrCout.Push("Main:total elapsed time. " + ss.str() );
	}
	_CrtDumpMemoryLeaks();
	(void)_getch();
	return 0;
}
```
### 実行結果
**コンソール**
```コンソール.
Sev:Enter __ReadFromCli                         0.000 msec // 1
Sev:Leave __ReadFromCli                         0.013 msec // 2
Sev:Enter __WriteToCli                          0.474 msec // 3
Sev:Server successfully wrote. "Hello, World!"  0.894 msec // 4
Sev:Leave __WriteToCli                          0.541 msec // 5
Sev:Enter operator>>                            0.548 msec // 6
Cli:Client successfully read."Hello, World!"    0.738 msec // 7
Sev:WriteToCliCompleted. "Hello, World!"        0.467 msec // 8
Cli:Client successfully wrote."Hello, World!"   0.549 msec // 9
Sev:ReadFromCliCompleted."Hello, World!"        0.581 msec // 10
Sev:Enter __ReadFromCli                         0.720 msec // 11
Sev:Leave __ReadFromCli                         0.589 msec // 12
Sev:Leave operator>>                            111.523 msec // 13
Main:"Hello, World!"                            0.998 msec // 14
Main:total elapsed time. 118.640 msec // 15
```
### mainソースコードの解説

**1. sampleIOCP s;**
　`sampleIOCP`オブジェクトを作成
**2. s << "Hello, World!"**
　パイプサーバー側にデータを送る。`sampleIOCP`オブジェクトが文字列を読み込み、名前付きパイプを通じてクライアント側へ書き込みをします。その後、内部では、クライアントが読み込み、クライアントからサーバーに書き込みをします。
**3. s.Await(100) >> str**
　クライアントから返ってきた文字列を**str**に書き込む。`Await(100)`は、100msec待つという意味です。クライアントがラグで直ぐに遅れない事に対応しています。
**4. s.OrCout.Push("Main:\\"" + str + "\\"")**
　結果を表示する。`s.OrCout.Push("Main:\"" + str + "\"");`の、`OrCout`は、シリアライズ化して順番を維持して表示する、OrderedCOutというクラスのオブジェクトです。前の表示との時間差を表示する機能もあります。
**5. s.OrCout.StopTimer()**
　OrCoutの計測タイマーをストップさせる
**6. s.OrCout.ShowTimeDisplay(false)**
　タイムを表示させる機能を停止。
**7. stringstream ss;// 7
ss << std::fixed << std::setprecision(3) << s.OrCout.TotalTime() << " msec";
s.OrCout.Push("Main:total elapsed time. " + ss.str() );**
　トータルタイムを表示させる。

:::note info
　このOrderedCOutクラスの解説は、今回は割愛します。
:::
### 以上、mainソースコードの解説終わりです

### 実行結果の表示の解説
#### 再掲載　コンソール
```コンソール.
Sev:Enter __ReadFromCli                         0.000 msec // 1
Sev:Leave __ReadFromCli                         0.013 msec
Sev:Enter __WriteToCli                          0.474 msec // 2
Sev:Server successfully wrote. "Hello, World!"  0.894 msec // 3
Sev:Leave __WriteToCli                          0.541 msec // 4
Sev:Enter operator>>                            0.548 msec // 5
Cli:Client successfully read."Hello, World!"    0.738 msec // 6
Sev:WriteToCliCompleted. "Hello, World!"        0.467 msec // 7
Cli:Client successfully wrote."Hello, World!"   0.549 msec // 8
Sev:ReadFromCliCompleted."Hello, World!"        0.581 msec // 9
Sev:Enter __ReadFromCli                         0.720 msec // 10
Sev:Leave __ReadFromCli                         0.589 msec // 11
Sev:Leave operator>>                            111.523 msec // 12
Main:"Hello, World!"                            0.998 msec // 13
Main:total elapsed time. 118.640 msec // 15
```
**1. Sev:Enter __ReadFromCli                         0.000 msec // 1
Sev:Leave __ReadFromCli                         0.013 msec**
　サーバー側、読み込み関数の中に入り、出ました。これで、予め、読み込みが可能になったら、**読み込み完了コールバック**が呼ばれるようにしておきます。**msec**は前の表示との時間差を表示します。一番最初の表示がタイマースタートのトリガーになります。
**2. Sev:Enter __WriteToCli                          0.474 msec // 2**
**Sev:Leave __WriteToCli                          0.541 msec // 4**
　サーバー側、書き込み関数の中に入り、出ました。
**3. Sev:Server successfully wrote. "Hello, World!"  0.894 msec // 3**
　書き込み動作が成功しました。

**4. Sev:Enter operator>>                            0.548 msec // 5
Sev:Leave operator>>                            111.523 msec // 12**
　出力オペレーターの関数内に入り、出ました。かなり間隔が空いているのが判ります。その間には、複数の他の操作が行われているのが見て取れます。また、**Leave**には**111.523 msec**と、時間がかかっているのが、判ります。これは、**Await**でタイムアウト時間を**100 msec**にした効果が現れているとみられます。

**5. Cli:Client successfully read."Hello, World!"    0.738 msec // 6**
　クライアント側、読み込み成功。
**6. Sev:WriteToCliCompleted. "Hello, World!"        0.467 msec // 7**
　サーバー側、書き込み完了コールバック関数が呼ばれました。
**8. Cli:Client successfully wrote."Hello, World!"   0.549 msec // 8**
　クライアント側、書き込み成功。
**9. Sev:ReadFromCliCompleted."Hello, World!"        0.581 msec // 9**
　サーバー側、読み込み完了コールバック関数が呼ばれました。
**10. Sev:Enter __ReadFromCli                         0.720 msec // 10
Sev:Leave __ReadFromCli                         0.589 msec // 11**
　サーバー側、読み込み完了コールバック関数の中で、次の読み込みが出来るようにこの関数が呼ばれています。　
**11. Main:"Hello, World!"                            0.998 msec // 13**
　main関数でクライアントからエコーされたデータを表示しています。成功したようです。
### 以上、表示の解説終わりです

## 普通の非同期と比べてみる
　普通の非同期はどんな動きをするか見てみましょう。
#### ../CommonLib/sampleIOCP.h
　`#define USING_IOCP`をコメントアウトして、ビルドします。
```../CommonLib/sampleIOCP.h
//#define USING_IOCP
```
### 実行結果
##### コンソール
```コンソール.
Sev:Enter __ReadFromCli                         0.000 msec
Sev:Leave __ReadFromCli                         0.011 msec
Sev:Enter __WriteToCli                          0.614 msec
Sev:Server successfully wrote. "Hello, World!"  0.489 msec
Sev:Leave __WriteToCli                          0.453 msec
Sev:Enter operator>>                            0.487 msec
Cli:Client successfully read."Hello, World!"    0.455 msec
Sev:WriteToCliCompleted. "Hello, World!"        0.516 msec
Cli:Client successfully wrote."Hello, World!"   1.795 msec
Sev:ReadFromCliCompleted."Hello, World!"        0.531 msec
Sev:Enter __ReadFromCli                         0.729 msec
Sev:Leave __ReadFromCli                         0.607 msec
Sev:Leave operator>>                            110.552 msec
Main:"Hello, World!"                            0.871 msec
Main:total elapsed time. 118.115 msec
```
　実は、内容に違いはありません。

## ここでちょっと実験
　バッファがパイプとカスタムオーバーラップ構造体にあるのですが、どのような影響が出るのか見てみたいと思います。
**../CommonLib/sampleIOCP.h**
```../CommonLib/sampleIOCP.h
class  sampleIOCP{
	static constexpr DWORD BUFFER_SIZE_OL = 0x10;
	static constexpr DWORD BUFFER_SIZE_CL_SIDE = 0x400;
	static constexpr DWORD BUFER_SIZE_PIPE = 0x100;
```
　このように**sampleIOCP**の宣言にバッファサイズが変更できるようになっています。これを変更します。**BUFFER_SIZE_CL_SIDE**は、クライアント側バッファサイズですが、これは変更しないで、十分用意されているものとして、サーバー側を見ていきます。
### 実験のコンディション
　リリースビルド、オプティマイズありにします。また、実行はコマンドプロンプトを立ち上げてその中でします。
### BUFFER_SIZE_OL = 0x10 -> BUFFER_SIZE_OL = 0x4
**結果コンソール**
```結果コンソール.
Sev:Enter __ReadFromCli                         0.000 msec
Sev:Leave __ReadFromCli                         0.004 msec
Sev:Enter __WriteToCli                          0.907 msec
Sev:Server successfully wrote. "Hell"           0.964 msec
Sev:WriteToCliCompleted. "Hell"                 1.058 msec
Sev:Server successfully wrote. "o, W"           0.821 msec
Sev:WriteToCliCompleted. "o, W"                 0.689 msec
Sev:Server successfully wrote. "orld"           0.691 msec
Sev:WriteToCliCompleted. "orld"                 0.792 msec
Sev:Server successfully wrote. "!"              0.688 msec
Sev:WriteToCliCompleted. "!"                    0.691 msec
Sev:Leave __WriteToCli                          0.697 msec
Sev:Enter operator>>                            0.682 msec
Cli:Client successfully read."Hello, World!"    0.686 msec
Cli:Client successfully wrote."Hello, World!"   4.667 msec
Sev:ReadFromCliCompleted."Hell"                 1.736 msec
Sev:Enter __ReadFromCli                         0.776 msec
Sev:ReadFromCliCompleted."o, W"                 0.816 msec
Sev:Enter __ReadFromCli                         0.751 msec
Sev:ReadFromCliCompleted."orld"                 0.672 msec
Sev:Enter __ReadFromCli                         0.684 msec
Sev:ReadFromCliCompleted."!"                    0.778 msec
Sev:Enter __ReadFromCli                         0.670 msec
Sev:Leave __ReadFromCli                         1.265 msec
Sev:Leave __ReadFromCli                         0.669 msec
Sev:Leave __ReadFromCli                         0.678 msec
Sev:Leave __ReadFromCli                         0.667 msec
Sev:Leave operator>>                            102.164 msec
Main:"Hello, World!"                            1.171 msec
Main:total elapsed time. 127.547 msec
```
　細かく分割されて交信していますね。でも正確に伝達できたようです。では、`BUFFER_SIZE_OL = 0x10`に戻して、次の実験をします。
### BUFER_SIZE_PIPE = 0x100->BUFER_SIZE_PIPE = 0x00
　これ、要るのか、要らないか、よく分からない設定でしたので、どうなるのでしょう。
**結果コンソール**
```結果コンソール.
Sev:Enter __ReadFromCli                         0.000 msec
Sev:Leave __ReadFromCli                         0.003 msec
Sev:Enter __WriteToCli                          1.616 msec
Sev:Server successfully wrote. "Hello, World!"  0.812 msec
Sev:Leave __WriteToCli                          0.873 msec
Sev:Enter operator>>                            0.670 msec
Cli:Client successfully read."Hello, World!"    0.677 msec
Sev:WriteToCliCompleted. "Hello, World!"        0.709 msec
Cli:Client successfully wrote."Hello, World!"   0.728 msec
Sev:ReadFromCliCompleted."Hello, World!"        0.804 msec
Sev:Enter __ReadFromCli                         0.987 msec
Sev:Leave __ReadFromCli                         0.880 msec
Sev:Leave operator>>                            107.734 msec
Main:"Hello, World!"                            1.450 msec
Main:total elapsed time. 117.949 msec
```
　特に違いはなさそうですね。
### 実験の結果
1. コードをうまく書いていれば、**BUFFER_SIZE_OL**バッファサイズは小さくても、大丈夫であった。
2. BUFER_SIZE_PIPEのサイズの差は見られなかった。

## 以上で実験を終わります
## 全体のまとめ
1. IOCPの知識が曖昧だったので、IOCPの流れが判るプログラムを作った。
2. パイプのタイプは**PIPE_TYPE_BYTE**（ストリーム）を選択。
3. 一応、動作は正常だった。
4. 以前作った「SubProcess」のプログラムは、やはり洗練されて無かった。できれば修正するのが、望ましい。
5. コーディングをし直すなど、手間がかかったが、理解が深まった。
# 終わりに
　「**[C++][Windows]IOCPを理解する**」の解説は以上となります。この記事が皆様の閃きや発想のきっかけになりましたら幸いです。
　また、ご意見、ご感想、ご質問など、お待ちしております。
