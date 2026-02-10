# ソフトウェア仕様書: rviz_util.h (詳細版)

## 1. 概要
`rviz_util.h` は、ROS 2 (rclcpp) 環境において、RViz上での3次元幾何形状（Marker）の可視化を簡略化する `TSimpleVisualizer` クラスを提供します。内部で `ay_cpp/geom_util.h` と連携し、複雑な姿勢計算を伴う図形描画をサポートします。

## 2. 基本情報
- **名前空間**: `trick`
- **作成者**: Akihiko Yamaguchi (info@akihikoy.net)
- **依存ライブラリ**: `rclcpp`, `visualization_msgs`, `geometry_msgs`, `Eigen 3`, `ay_cpp/geom_util.h`
- **注意**: リアルタイムスレッド等からの同時アクセス（スレッドセーフ）は考慮されていません。

---

## 3. 重要：安全管理・メモリ制約
本クラスを利用する際は、以下のリスクを呼び出し側で管理する必要があります。

1. **配列・ポインタのリスク**:
   - `AddCylinder` 等の内部では、`geom_util.h` の生配列操作関数（`GPointToP` 等）を呼び出します。引数に渡すメッセージ型が不適切な場合、メモリ不正アクセスが発生します。
2. **IDの競合**:
   - `mid` を手動指定する場合、既存の `curr_id_` と重複すると表示が上書きされます。自動採番を利用する場合は `Reset()` のタイミングに注意してください。
3. **特異点のリスク**:
   - 2点間を結ぶ図形（Cylinder）において、点間距離が 0 の場合、内部の姿勢計算で **NaN** が発生し、RViz上での描画が破綻します。

---

## 4. 公開関数リファレンス (TSimpleVisualizer)

#### **Setup**
- **引数**: `node`, `viz_dt`, `name_space`, `frame`, `queue_size`, `topic`
- **内容**: パブリッシャーを初期化し、QoS（Reliable / Transient Local）を設定します。
- **リスク**: `node` が `nullptr` の場合、即座にクラッシュします。

#### **AddArrow / AddCube / AddSphere**
- **引数**: `Pose x`, `Vector3 scale`, `ColorRGBA rgb`, `float alpha`, `int mid=-1`
- **内容**: 単一の基本図形を描画します。`mid` 未指定時は自動採番されます。

#### **AddCylinder**
- **引数**: `Point p1, p2`, `float diameter`, `ColorRGBA rgb`, `float alpha`, `int mid=-1`
- **内容**: $p1$ と $p2$ を結ぶ円柱を描画します。
- **リスク**: $p1 = p2$ の場合、内部の `XFromP1P2` でゼロ除算が発生し、姿勢データが `NaN` となります。

#### **AddPoints**
- **引数**: `const std::vector<Point> &points`, `Vector3 scale`, `ColorRGBA rgb`, `float alpha`, `int mid=-1`
- **内容**: 指定された点群（`POINTS`）を一括で描画します。
- **特異事項**: 個々の点の姿勢は考慮されず、全体が原点基準のマーカーとして送信されます。

#### **AddPolygon**
- **引数**: `const std::vector<Point> &points`, `Vector3 scale`, `ColorRGBA rgb`, `float alpha`, `int mid=-1`
- **内容**: `LINE_STRIP` を使用して、点群を順に結ぶ折れ線を描画します。
- **リスク**: 頂点数が 1 つの場合、描画されません。

#### **AddLineList**
- **引数**: `const std::vector<Point> &points`, `Vector3 scale`, `ColorRGBA rgb`, `float alpha`, `int mid=-1`
- **内容**: `LINE_LIST` を使用して、2点ずつのペア（$p_0$-$p_1, p_2$-$p_3, \dots$）を独立した線分として描画します。
- **リスク**: `points.size()` が奇数の場合、最後の 1 点は無視されます。

#### **DeleteMarker / DeleteAllMarkers**
- **内容**: 特定のIDまたは現在の名前空間内の全マーカーの削除コマンド（`DELETE` / `DELETEALL`）を送信します。

#### **Reset**
- **内容**: 内部の ID カウンタを 0 に戻します。

---

## 5. クラス仕様: TSimpleVisualizerArray (MarkerArray版)
`TSimpleVisualizer` を継承し、複数のマーカーをバッファリングして一括送信するためのクラスです。

#### **Setup (Override)**
- **引数**: `node`, `viz_dt`, `name_space`, `frame`, `queue_size`, `topic`
- **内容**: `MarkerArray` 型のパブリッシャーを初期化します。親クラスのトピック送出機能は無効化（空文字列指定）されます。

#### **marker_operation (Override)**
- **内容**: `Add...` 系関数が呼ばれた際、マーカーを即座に送信せず、内部の `marker_array_` リストに追加（蓄積）します。

#### **Publish**
- **内容**: 蓄積された `MarkerArray` を一括でパブリッシュし、その後 `Reset()` を呼び出して内部バッファをクリアします。
- **利点**: 個別にパブリッシュするよりも通信オーバーヘッドが大幅に削減されます。

#### **Reset (Override)**
- **内容**: ID カウンタのリセットに加え、蓄積された `marker_array_` を空にします。

#### **DeleteAllMarkers (Override)**
- **内容**: `DELETEALL` アクションを持つマーカーをバッファに追加し、即座に `Publish()` を実行して RViz 上の表示をクリアします。


---

## 6. 通信およびライフタイム

- **マーカーの寿命 (`viz_dt_`)**:
  - `Setup` または `SetDt` で設定された時間は、全ての新規追加マーカーの `lifetime` メンバに適用されます。
  - デストラクタ実行時に `viz_dt_` が 0 以外であれば、自動的に `DeleteAllMarkers` が発行されます。
- **パブリッシュのタイミング (TSimpleVisualizer)**:
  - 各 `Add...` 関数は呼び出された瞬間に `publish()` を実行します。数千個のマーカーを個別に描画すると通信負荷が急増するため、大量の線分や点群を扱う場合は `AddLineList` や `AddPoints` を優先的に使用してください。

## 7. 安全管理・リスク詳細 (TSimpleVisualizerArray)

#### **1. MarkerArray 使用時の注意**
- **送信タイミング**: `Add...` を呼ぶだけでは RViz に反映されません。必ず最後に `Publish()` を呼び出す必要があります。
- **メモリ消費**: `Publish()` を呼ぶまでマーカーがメモリ上に蓄積されるため、ループ内で `Publish()` を忘れるとメモリ消費が際限なく増加します。

#### **2. ID の管理と上書き**
- `Reset()` により ID が 0 に戻るため、`Publish()` ごとに同じ ID が再利用されます。これにより、以前のフレームのマーカーが新しい位置で適切に更新されます。

#### **3. ゼロ除算・NaN のリスク**
- `AddCylinder` の 2 点間距離が 0 の際のリスクは親クラスと同様です。`NaN` を含むマーカーが `MarkerArray` に混入すると、RViz 側のレンダリングエラーを引き起こす可能性があります。

#### **4. スレッドセーフティ**
- `marker_array_`（`std::vector`）への追加操作が行われるため、複数のスレッド（例：タイマーコールバックとメインループ）から同時に描画関数を呼び出すと、致命的なメモリ破壊を招く恐れがあります。

---

## 8. 推奨される利用パターン (TSimpleVisualizerArray)

```cpp
// 1. ループの先頭で描画準備
// (注: TSimpleVisualizerArray の場合は Add 前に Reset は自動で行われないため Publish 後の Reset が重要)

// 2. 形状の追加
visualizer.AddSphere(p1);
visualizer.AddArrow(pose2);

// 3. 一括送信
visualizer.Publish(); // この内部で Reset() も行われる
```
