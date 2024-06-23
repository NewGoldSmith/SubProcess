# この記事の対象読者
　Windows APIを使う際、「ああ、メモリープールがほしい･･･。」と思い、メモリープールを作り、それを説明した記事です。ソースコードの修正次第ではWindows非依存にする事も可能です。
## 固定サイズのメモリーブロックを何度も使いまわす、メモリープール。
　Windowsの非同期IOは、OVERLAPPED構造体(実際は、ほとんどこれを包含した構造体）を使います。これは、ノンプリエンティブシングルスレッドでもサーバー用途に使えるトリッキーな仕組みなんですが、今はもうデザインが古いですよね。
　例えば、サーバー用途でクライアントの待ち受けに使う場合、１接続毎にこのOVERLAPPED構造体が必要で、この構造体へのアドレスをWindowsシステムに引数として渡します。この構造体はID検出の仕組みも兼ねていて、ユーザーが任意のID検出の仕組みを仕込んでおけば、どのクライアントからメッセージが届いたかも知る事が出来ます。サーバー用途では、大量にこのOVERLAPPED構造体が必要になります。また、この構造体のライフタイムはクライアントの都合に合わせて、様々になります。その都度、メモリを動的確保、動的開放するのが手っ取り早い使い方なのですが、new、deleteはそれなりにコストがかかります。そこで、構造体のサイズが予め判っているのなら、それをプールしておこうという発想がメモリプールです。
## スレッドプールにも必須？メモリープール
　折角、自分のパソコンのCPUに複数のコアがあるなら、目いっぱい使いたい、そう思ったことは無いでしょうか？「ただ、プログラムを作ったら、OSが勝手に複数のコアで仕事してくれるのじゃないの？」。そんなわけありません。「Pythonが裏で凄い事やってるよ。」。･･･そんな人任せのライブラリを使うだけでいいのですか･･･？
　と、いうわけでパソコンのCPUを骨までしゃぶるプログラムを作って行こうというのなら、スレッドプールを使う事になります。スレッドプールならCPUを遊ばせない効率よいプログラムが書けます。効率が上がればOVERLAPPED構造体やデータなど、スレッドに渡すメモリー領域はテンポよく供給する必要があります。プールしておいた構造体を供給するのなら、テンポよく供給できます。
## さて、１秒間に何万回もnew出来るのか？
 　Visual Studio 2022（以下VS）には、instrumentationというプロファイリング機能があります。これは、呼び出した関数の回数や積算時間を正確にカウントできる機能です。ただ、アンドキュメントなところがあって、関数を数万回呼び出しを行うプログラムを実行すると、プログラムが途中で終了してしまいます。同じプログラムを使って呼び出す回数を減らす操作をした場合、途中で終了しないので、関数呼び出し回数と途中終了は関係がありそうです。
　マルチコア環境でのスレッドプールのスループットを計測すると、１秒間に数万回の呼び出しは可能です。以前、木構造のノードをメモリープールのユニットで保持した事があるのですが、**数秒間で1000万ユニット以上の延べ使用数**を計測した事もありました。このことから、ワークアイテムをキューに入れる際、その都度newするのはいかがな事かと思われます。
## 仕組みは配列の要素アドレスのリングバッファ
　ですので、ユニットアドレスが供給元の配列の端でない場合、前後のアドレスにもアクセス出来ますし、アドレスを返却した後にもアクセス出来ますので、使用者が規律を持って運用する必要があります。
## テンプレートstd::コンテナクラスでもメモリープールは作れる？
　はい、作れます。むしろ、こちらの方がいいかもしれません。カスタムアロケーターを作れば、メモリーをスレッドローカルで確保するのか、プロセスローカルで確保するのか、メモリー戦略の幅が広がります。ただ、マルチスレッドで使う場合、ロックする機構が必要になるでしょうから、そういうのを内包したクラスを作る事になるでしょう。メモリープールを作り始めた頃は、stdライブラリの使用に不慣れだったこともあり、採用はしませんでした。
## MemoryLoanテンプレートクラスの使い方のデモコード
### この記事で使うソースコードへのリンク
[GitHubへのリンクはここです。Visual Studio 2022用に設定されたslnファイルもあります。](https://github.com/NewGoldSmith/SubProcess "https://github.com/NewGoldSmith/SubProcess")
　TestProjectをスタートアッププロジェクトに設定し、ソリューションエクスプローラーから**test.cpp**を選択し、プロパティの設定で**全般->ビルドから除外**項目を**いいえ**に設定し、**test.cpp以外**は**はい**に設定し、ターゲットCPUをx64に設定し、`F5`を押下すると実行できます。
![](https://storage.googleapis.com/zenn-user-upload/f736f8852a36-20240623.png)

　ソースコードの中にはデバッグ用のライブラリも含んでいます。本質ではない為、今回は説明を割愛いたします。

>　この記事で紹介しているソースコードは、公開した時点から変更を加えている事があります。そのため、元の記事とは異なる結果を得る場合があります。また、ソースコードを機能別にディレクトリを分ける等の、改善を行う可能性があります。
 
### デモコード
 　下記にデモコードを記載します。その後、番号のコメントが付けられているところの、解説を順次行います。
```test.cpp
// test.cpp
#include <iostream>
#include "../CommonLib/MemoryLoan.h"

using namespace std;

int main() {
	{
		int iArr[1]{};//1
		MemoryLoan<int> mp(iArr, sizeof(iArr) / sizeof(iArr[0]));//2
		mp.DebugString("test1");//3
		int *pi1 = mp.Lend();//4
		*pi1 = 100;//5
		cout << *pi1 << endl;//6
		mp.Return(pi1);//7
		int *pi2 = mp.Lend();//8
		*pi2 = 200;//9
		cout << *pi2 << endl;//10
		mp.Return(pi2);//11
		int iArr2[2]{};//12
		mp.ReInitialized(iArr2, sizeof(iArr2) / sizeof(iArr2[0]));//13
		mp.DebugString("test2");//14
		pi1 = mp.Lend();//15
		pi2 = mp.Lend();//16
		*pi1 = 250;//17
		*pi2 = *pi1 + 50;//18
		cout << *pi1 << endl << *pi2 << endl;//19
		mp.Return(pi1);//20
		mp.Return(pi2);//21
	}
	_CrtDumpMemoryLeaks();
}
```
#### デモコードの解説
##### １、メモリープールに使うメモリー領域を確保する
```test.cpp
		int iArr[1]{};//1
```
　このメモリープールテンプレートクラスは使用するメモリーを確保する機能が搭載されていません。この辺りはstdライブラリとは相容れない仕様です。１ユニット確保しています。このメモリープールは、`int`等のプリミティブな型だけでなく、クラスの配列等も取り扱う事ができます。
##### ２、コンストラクタでメモリープールオブジェクトを構築する
```test.cpp
		MemoryLoan<int> mp(iArr, sizeof(iArr) / sizeof(iArr[0]));//2
```
　コンストラクタはデフォルトコンストラクタを禁止にしています。代わりに`MemoryLoan(T *const pBufIn, size_t sizeIn)`を使います。`pBufIn`は配列のアドレス、`sizeIn`はその配列の要素数となっています。
##### ３、デバッグ文字列をセットする
```test.cpp
		mp.DebugString("test1");//3
```
　これは任意の設定です。これを設定しておくと、デストラクタが呼ばれた時に、デバッグ出力に設定した文字列が表示されます。複数のメモリープールオブジェクトを使っている時に、どのオブジェクトのデストラクタが呼ばれたのか、識別できるようにしています。
　また、何らかの例外が発生したときも、デバッグ出力にこの文字列を表示するようになっています。
##### ４、貸し出しをする
```test.cpp
int *pi1 = mp.Lend();//4
```
　配列の要素のアドレスを返します。
##### ５、６、何らかの作業をする
```test.cpp
		*pi1 = 100;//5
		cout << *pi1 << endl;//6
```
　ここでは代入し、コンソールに出力しています。
##### ７、返却する
```test.cpp
		mp.Return(pi1);//7
```
　借りていたユニットを返却しています。
##### ８、９、１０、１１、また借りて作業をして返却する
```test.cpp
		int *pi2 = mp.Lend();//8
		*pi2 = 200;//9
		cout << *pi2 << endl;//10
		mp.Return(pi2);//11
```
　また借ります。
##### １２、１３、１４、別のメモリー領域をセットして再初期化する。
```test.cpp
		int iArr2[2]{};//12
		mp.ReInitialized(iArr2, sizeof(iArr2) / sizeof(iArr2[0]));//13
		mp.DebugString("test2");//14
```
　配列のサイズを変えたいので、それを指定して、再初期化します。再初期化するとデバッグ出力に、直前に使用していた統計をデバッグ出力に表示します。
##### １５～２１、新たな領域で作業をし返却する
```test.cpp
		pi1 = mp.Lend();//15
		pi2 = mp.Lend();//16
		*pi1 = 250;//17
		*pi2 = *pi1 + 50;//18
		cout << *pi1 << endl << *pi2 << endl;//19
		mp.Return(pi1);//20
		mp.Return(pi2);//21
```
　ユニットサイズを`ReInitialized`で増やしたので、新しい作業も問題なく行えています。
##### コンソール結果
```コンソール.結果
100
200
250
300
```
## MemoryLoanテンプレートクラスリファレンス
### 冒頭での条件指定
　冒頭で条件指定が、出来るようになっています。必要に応じて`#define`をコメントアウト、アンコメントして下さい。
```MemoryLoan.h
// ********使用条件を設定***********
#define ML_USING_CRITICAL_SECTION
#define ML_CONFIRM_RANGE
#define ML_USING_DEBUG_OUT
#define ML_USING_STD_ERROR
// ******条件設定終わり*************
```
#### ML_USING_CRITICAL_SECTION
　クリティカルセクションを使用します。複数のスレッドで同時使用する可能性がある場合、この機能を使います。
#### ML_CONFIRM_RANGE
　貸出し過多、返却過多になっていないか確認します。なっていた場合、例外を投げます。
#### ML_USING_DEBUG_OUT
　デストラクタが呼ばれた時に、デバッグ出力に情報を出力します。
>MemoryLoan is destructing. DebugMessage:"test2" TypeName:"int" BytesPerUnit:4bytes TotalNumberOfLoans:2 TotalNumberOfReturns:2 NumberOfUnreturned:0 NumberOfUnits:2 MaximumNumberOfLoans:2

の様にデバッグ出力されます。それぞれの内容の意味は次のようになります。
##### DebugMessage:"test2"
　予めデバッグメッセージを`DebugString`メソッドで文字列を仕組んでおくと、**""**の間にその文字列が表示されます。インスタンスを複数作った場合の識別にも使えます。
##### TypeName:"int"
　型名を表示します。
##### BytesPerUnit:4bytes
　１ユニット辺りのメモリー使用量を表示します。
##### TotalNumberOfLoans:2
　総貸出数を表示します。
##### TotalNumberOfReturns:2
　総返却数を表示します。
##### NumberOfUnreturned:0
　デストラクタが呼ばれた時の、未返却数を表示します。
##### NumberOfUnits:2
　ユニット数を表示します。
##### MaximumNumberOfLoans:2
　ピークの最大貸出し数を表示します。
#### ML_USING_STD_ERROR
　エラー出力をcerrに出力します。

 ***複数の条件で複数のインスタンスを使用したい場合どうするの？***
>　MemoryLoan.hをコピーして、違うファイル名にして保存し、クラス名も被らない名前に変更します。これで複数の条件で使用できます。スマートではないですけどいい方法が思いつかないのでこの方法にしています。もちろん条件を変数で持って、切り替える事は出来るでしょうけど、ヘッダを書き換えるだけだからこれでいいかなと思っています。クラス名の変更ですが、Visual Studio 2022（以下VS）ならば、カーソルをクラス名に持って行き、キーボードショートカット`Ctrl + r, r`で変更できます。MemoryLoan.hの末尾に
```MemoryLoan.h
#undef ML_USING_CRITICAL_SECTION
#undef ML_CONFIRM_RANGE
#undef ML_USING_DEBUG_OUT
#undef ML_USING_STD_ERROR
```
>の様に、#defineを#undefしていますので、各ファイルごとに違う設定が可能です。

### メンバー関数及び、コード解説
ここからはメンバー関数及び、コード解説です。クラス名は**MemoryLoan**です。

#### MemoryLoan(T *const pBufIn, size_t sizeIn)
##### 説明
　コンストラクタ
##### 引数
###### pBufIn
  　配列のアドレス。
###### sizeIn
　配列の要素数。

```MemoryLoan.h
	MemoryLoan(T *const pBufIn, size_t sizeIn)
		:ppBuf(nullptr)
		, front(0)
		, end(0)
		, mask(sizeIn - 1)
	{
	// @attention sizeInは2のべき乗でなくてはなりません。
		ppBuf = new T * [sizeIn];
		for (size_t i(0); i < sizeIn; ++i) {
			ppBuf[i] = &pBufIn[i];
		}
	}

```
　原理部分だけを抜き出したコードです。配列の各要素のアドレスを格納する配列を`new`で確保し、`ppBuf`に格納しています。スレッドローカルにするならばこの辺の改善も出来ます。メンバーは配列アドレスを格納する`ppBuf`、リングバッファの先頭を記録する`front`、末尾を記録する`end`、アドレスをマスクして同じアドレスをグルグル回る様にする為の、`mask`を初期化しています。`mask`は`配列の要素数-1`ですので、配列の要素数を知りたいときにも使います。**要素数が2のべき乗でない場合、発見困難なバグに遭遇する可能性**がありますので十分注意が必要です。
#### ~MemoryLoan()
##### 説明
デストラクタ。
```MemoryLoan.h
	~MemoryLoan() {
		delete[] ppBuf;
	}
```
　`ppBuf`を`delete`しています。
#### void ReInitialized(T *pBufIn, size_t sizeIn)
##### 説明
　再初期化をします。
##### 引数
###### T *pBufIn
　T型の配列のアドレス。
###### size_t sizeIn
　配列のユニット数。
 ```MemoryLoan.h
 	// @attention sizeInは2のべき乗でなくてはなりません。
	void ReInitialized(T *pBufIn, size_t sizeIn) {
		delete[] ppBuf;
		front = 0;
		end = 0;
		mask = sizeIn - 1;
		ppBuf = new T * [sizeIn];
		for (size_t i(0); i < sizeIn; ++i) {
			ppBuf[i] = &pBufIn[i];
		}
	}
```
　まず、再初期化する事は無いと思われますが、`コピーコンストラクタ`、`moveコンストラクタ`が使えないので、それの代わりになる物を用意しました。実装しようと思えばできそうですが、今の所、必要ないので実装していません。当然`std::vecotr`等の要素には使えません。
#### T *Lend()
##### 説明
　貸出しをします。
##### 戻り値
###### T *
　T型のユニットのポインタを返します。
```MemoryLoan.h
	inline T *Lend() {
		T **ppT = &ppBuf[end & mask];
		++end;
		return *ppT;
	}
```
　貸出しをして、`end`をインクリメントしています。複数の連続した要素を扱う事は出来ません。
#### void Return(T *const pT)
##### 説明
　返却を受け付けます。
##### 引数
###### T *const pT
　T型の返却するポインタ。
 ```MemoryLoan.h
	inline void Return(T *const pT) {
		ppBuf[front & mask] = pT;
		++front;
	}
```
　返却をして`front`をインクリメントしています。
#### void DebugString(const std::string &str)
##### 説明
　デバッグ用の文字列を設定します。
##### 引数
###### const std::string &str
　設定する文字列。
  ```MemoryLoan.h
	void DebugString(const std::string &str) {
		strDebug = str;
	}
```
　デストラクタが呼ばれた時、デバッグ出力に文字列を出力します。どのオブジェクトのデストラクタが呼ばれたか、識別可能にする事が出来ます。
## 以上、リファレンス終わり

>　SubProcessクラスは、機能向上の為、改良を加える事があります。また、追加のメンバーが加えられる可能性もあります。

# 終わりに
　「[C++]超高速テンプレートメモリープールコンテナの実装」の解説は以上となります。この記事が皆様の閃きや発想のきっかけになりましたら幸いです。
 　また、ご意見、ご感想、ご質問など、お待ちしております。

