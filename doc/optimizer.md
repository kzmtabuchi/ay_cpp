# ソフトウェア仕様書: optimizer.h / optimizer.cpp

## 1. 概要
`optimizer.h` は、進化戦略の一種である **CMA-ES (Covariance Matrix Adaptation Evolution Strategy)** アルゴリズムを用いて、目的関数の最小化を行う最適化ライブラリです。境界条件（制約条件）を伴う実数値最適化問題をサポートします。

## 2. 基本情報
- **ファイル名**: `optimizer.h`, `optimizer.cpp`
- **名前空間**: `trick`
- **作成者**: Akihiko Yamaguchi (info@akihikoy.net)
- **バージョン**: 0.1
- **最終更新日**: 2016年6月28日
- **依存ライブラリ**: 
  - `boost/function.hpp`
  - 3rdparty CMA-ES ライブラリ (`cmaes_interface.h`, `boundary_transformation.h`)

---

## 3. 公開構造体: TCMAESParams

CMA-ESアルゴリズムの挙動を制御するパラメータ群です。

#### **主要メンバ変数**
- **`int lambda`**:
  - 各世代の個体数（個体群サイズ）。`0` を指定するとデフォルト値が自動設定されます。
- **`double stopMaxFunEvals`**:
  - 最大関数評価回数。この回数に達すると最適化が停止します（デフォルト: 22500）。
- **`double stopTolFun / stopTolFunHist`**:
  - 目的関数の改善幅に基づく停止閾値。
- **`int PrintLevel`**:
  - コンソール出力レベル。`0`: 出力なし、`1`: 標準、`2`: 詳細。

---

## 4. 公開関数: MinimizeF

目的関数 `f` を最小化するパラメータを探索します。

#### **引数仕様**
- **`boost::function<double(const double x[], bool &is_feasible)> f`**:
  - 最小化対象の目的関数。
  - `x`: 入力パラメータ配列、`is_feasible`: 入力値が実行可能（制約を満たす）かを出力するフラグ。
- **`double x0[]`**: 初期探索点（サイズ `dim`）。
- **`double sig0[]`**: 各次元の初期標準偏差（ステップサイズ、サイズ `dim`）。
- **`int dim`**: 探索空間の次元数。
- **`const double xmin[], const double xmax[]`**: 各次元の最小値および最大値の境界配列。
- **`int bound_len`**: 境界配列の長さ。
- **`double *xres`**: 探索結果（最良解）の出力先。**※ユーザー側で `dim` 個分のメモリ確保が必要です。**
- **`const TCMAESParams &params`**: 最適化パラメータ。

#### **内部処理と特徴**
1. **境界変換**:
   CMA-ES内部の探索空間を実際の `xmin`, `xmax` の範囲に写像するための `boundary_transformation` を適用します。
2. **実行可能領域の再サンプリング**:
   目的関数内で `is_feasible = false` が返された場合、個体の再サンプリング (`cmaes_ReSampleSingle`) を行い、実行可能な点が見つかるまで試行します。
3. **終了条件**:
   アルゴリズムの収束（停止閾値）または `stopMaxFunEvals` への到達によって終了します。
4. **出力**:
   `PrintLevel >= 1` の場合、`cmaes_signals.par` 等のログファイルや標準出力に途中経過を出力します。

---

## 5. 安全管理・メモリ・制約上のリスク

#### **1. メモリ管理**
- **出力配列 `xres`**: 関数内部でメモリ確保は行われません。呼び出し側で `dim` 要素以上の領域が確保されたポインタを渡してください。不適切なポインタはバッファオーバーランを招きます。
- **内部動的確保**: 内部で `cmaes_init` 等によるメモリ確保が行われますが、これらは `cmaes_exit` および `free` によって関数終了時に適切に解放されます。

#### **2. 特異点・計算負荷のリスク**
- **制約条件のループ**: 目的関数が `is_feasible = false` を返し続けるような極めて狭い実行可能領域の場合、`while(true)` ループ内で計算が停滞し、`stopMaxFunEvals` に達するまで抜けない可能性があります。
- **境界設定**: `xmin` と `xmax` の幅が初期ステップサイズ `sig0` に対して極端に狭い場合、アルゴリズムが適切に収束しない恐れがあります。

#### **3. 依存関係のリスク**
- 本関数は `ay_cpp/3rdparty/cma_es` 下の C 言語実装のインターフェースに依存しています。ビルド時にはこれらのライブラリおよび `boundary_transformation.h` が正しくリンクされている必要があります。

#### **4. 並列実行**
- 内部で一時ファイル（`cmaes_signals.par`, `/tmp/allcmaes.dat`）への書き出しが行われるため、同一ディレクトリ・同一ユーザーで複数の最適化を同時に走らせると、ファイル競合が発生する可能性があります。

---

## 6. 使用例（疑似コード）

```cpp
bool MyFunc(const double x[], bool &is_feasible) {
  is_feasible = (x[0] > 0); // 例：制約条件
  return x[0]*x[0] + x[1]*x[1]; // 最小化したい値
}

// ...
double xres[2];
trick::TCMAESParams params;
params.stopMaxFunEvals = 1000;
trick::MinimizeF(MyFunc, x0, sig0, 2, xmin, xmax, 2, xres, params);
```
