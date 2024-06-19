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
## 実際のソースコードの紹介
[ソースコードはここです。](https://github.com/NewGoldSmith/Memory-Pool/tree/main "https://github.com/NewGoldSmith/Memory-Pool/tree/main")
メモリープール動作デモmain関数を作りました。
### 使い方　デモコード
```test.cpp
// test.cpp
#include <iostream>
#include "MemoryLoan.h"

using namespace std;

int main() {
	{
		int iArr[1]{};
		MemoryLoan<int> mp(iArr, sizeof(iArr) / sizeof(iArr[0]));
		mp.DebugString("test1");
		int *pi1 = mp.Lend();
		*pi1 = 100;
		cout << *pi1 << endl;
		mp.Return(pi1);
		int *pi2 = mp.Lend();
		*pi2 = 200;
		cout << *pi2 << endl;
		mp.Return(pi2);
		int iArr2[2]{};
		mp.ReInitialized(iArr2, sizeof(iArr2) / sizeof(iArr2[0]));
		mp.DebugString("test2");
		pi1 = mp.Lend();
		pi2 = mp.Lend();
		*pi1 = 250;
		*pi2 = *pi1 + 50;
		cout << *pi1 << endl << *pi2 << endl;
		mp.Return(pi1);
		mp.Return(pi2);
	}
	_CrtDumpMemoryLeaks();
}
```
### MemoryLoan.hテンプレートクラスの解説
##### 冒頭での条件指定
冒頭で条件指定出来るようにしています。必要に応じて`#define`をコメントアウト、アンコメントして下さい。
```MemoryLoan.h
// ********使用条件を設定***********
#define ML_USING_CRITICAL_SECTION
#define ML_CONFIRM_RANGE
#define ML_USING_DEBUG_OUT
#define ML_USING_STD_ERROR
// ******条件設定終わり*************
```
<dl>
  <dt>ML_USING_CRITICAL_SECTION</dt>
  <dd>クリティカルセクションを使用します。複数のスレッドで同時使用する可能性がある場合、この機能を使います。</dd>
  <dt>ML_CONFIRM_RANGE</dt>
  <dd>貸出し過多、返却過多になっていないか確認します。なっていた場合、例外を投げます。</dd>
  <dt>ML_USING_DEBUG_OUT</dt>
  <dd>デストラクタが呼ばれた時にデバッグ出力に情報を出力します。<br>
  <blockquote>MemoryLoan is destructing. DebugMessage:"test2" TypeName:"int" BytesPerUnit:4bytes TotalNumberOfLoans:2 TotalNumberOfReturns:2 NumberOfUnreturned:0 NumberOfUnits:2 MaximumNumberOfLoans:2</blockquote>
  </dd>
</dl>
<dl>
  <dt>DebugMessage:"test2"</dt>
  <dd>予めデバッグメッセージを<code>DebugString</code>メソッドで文字列を仕組んでおくと、<strong>""</strong>の間にその文字列が表示されます。インスタンスを複数作った場合の識別にも使えます。</dd>
  <dt>TypeName:"int"</dt>
  <dd>型名を表示します。</dd>
  <dt>BytesPerUnit:4bytes</dt>
  <dd>１ユニット辺りのメモリー使用量を表示します。</dd>
  <dt>TotalNumberOfLoans:2</dt>
  <dd>総貸出数を表示します。</dd>
  <dt>TotalNumberOfReturns:2</dt>
  <dd>総返却数を表示します。</dd>
  <dt>NumberOfUnreturned:0</dt>
  <dd>デストラクタが呼ばれた時の、未返却数を表示します。</dd>
  <dt>NumberOfUnits:2</dt>
  <dd>ユニット数を表示します。</dd>
  <dt>MaximumNumberOfLoans:2</dt>
  <dd>ピークの最大貸出し数を表示します。</dd>
</dl>
<dl>
  <dt>ML_USING_STD_ERROR</dt>
  <dd>エラー出力をcerrに出力します。</dd>
</dl>

#### 複数の条件で複数のインスタンスを使用したい場合どうするの？
　MemoryLoan.hをコピーして、違うファイル名にして保存し、クラス名も被らない名前に変更します。これで複数の条件で使用できます。スマートではないですけどいい方法が思いつかないのでこの方法にしています。もちろん条件を変数で持って、切り替える事は出来るでしょうけど、ヘッダを書き換えるだけだからこれでいいかなと思っています。クラス名の変更ですが、Visual Studio 2022（以下VS）ならば、カーソルをクラス名に持って行き、キーボードショートカット`Ctrl + r, r`で変更できます。MemoryLoan.hの末尾に

```MemoryLoan.h
#undef ML_USING_CRITICAL_SECTION
#undef ML_CONFIRM_RANGE
#undef ML_USING_DEBUG_OUT
#undef ML_USING_STD_ERROR
```

の様に、#defineを#undefしていますので、各ファイルごとに違う設定が可能です。
#### メンバー関数及び、コード解説
ここからはメンバー関数及び、コード解説です。クラス名は**MemoryLoan**です。

##### コンストラクタ
`MemoryLoan(T *const pBufIn, size_t sizeIn)`
<dl>
  <dt>pBufIn</dt>
  <dd>　配列のアドレス。</dd>
  <dt>sizeIn</dt>
  <dd>　配列の要素数。</dd>

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
#### デストラクタ
`~MemoryLoan()`
```MemoryLoan.h
	~MemoryLoan() {
		delete[] ppBuf;
	}
```
　`ppBuf`を`delete`しています。
 #### 再初期化
 `void ReInitialized(T *pBufIn, size_t sizeIn)`
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
#### 貸出し
`T *Lend()`
```MemoryLoan.h
	inline T *Lend() {
		T **ppT = &ppBuf[end & mask];
		++end;
		return *ppT;
	}
```
　貸出しをして、`end`をインクリメントしています。複数の連続した要素を扱う事は出来ません。
 #### 返却
 `void Return(T *const pT)`
 ```MemoryLoan.h
	inline void Return(T *const pT) {
		ppBuf[front & mask] = pT;
		++front;
	}
```
　返却をして`front`をインクリメントしています。
 #### デバッグ用文字設定
 `void DebugString(const std::string &str)`
  ```MemoryLoan.h
	void DebugString(const std::string &str) {
		strDebug = str;
	}
```
　デストラクタが呼ばれた時、デバッグ出力に文字列を出力します。どのオブジェクトのデストラクタが呼ばれたか、判る様にする事が出来ます。
 # 終わりに
　「[C++]超高速テンプレートメモリープールコンテナの実装」の解説は以上となります。この記事が皆様の閃きや発想のきっかけになりましたら幸いです。
 　また、ご意見、ご感想、ご質問など、お待ちしております。

