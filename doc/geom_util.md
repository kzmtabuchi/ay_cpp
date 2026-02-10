# ソフトウェア仕様書: geom_util.h

## 1. 概要
`geom_util.h` は、Eigen ライブラリおよび（ビルド設定により）ROS の geometry_msgs を利用して、
3次元幾何計算（座標変換、姿勢変換、ベクトル操作、簡易的な幾何生成・解析）を行うための
C++ ヘッダライブラリです。


## 2. 基本情報
- **ファイル名**: geom_util.h
- **名前空間**: `trick`
- **作成者**: Akihiko Yamaguchi (info@akihikoy.net)
- **依存ライブラリ**: [Eigen 3](https://eigen.tuxfamily.org) (Core, Geometry)
- **注意**: パフォーマンスを最優先しており、内部での配列境界チェックやNULLポインタの検証は一切行われません。


## 3. 安全管理・メモリ制約
本ライブラリを使用する際、呼び出し側は以下の制約を厳守する必要があります。不遵守は**セグメンテーションフォールト（強制終了）**や**未定義の動作**を招きます。

1. **配列長の保証**: 各関数が要求する要素数（3, 4, 6, 7要素）以上の有効なメモリ領域を指すポインタを渡してください。
2. **NULLポインタ参照のリスク**: `fault` や `preferable` などのポインタ引数に `NULL` を渡すと、特定の条件（特異点など）でポインタ参照が発生し、クラッシュします。
3. **特異点のリスク**: ゼロベクトルや平行なベクトルが入力された場合、内部でゼロ除算（`v.normalized()`等）が発生し、結果に `NaN`（非数）が混入します。

---

## 4. データ構造 (構造体)
STLコンテナ等での利用を想定した、固定長配列をラップする構造体です。

### 4.1. TPos, TPose, TPosVel

| 構造体名 | 内容 | 配列仕様 |
| :--- | :--- | :--- |
| `TPos` | 3次元位置 | `double X[3]` (x, y, z) |
| `TPose` | 3次元姿勢 | `double X[7]` (x, y, z, qx, qy, qz, qw) |
| `TPosVel` | 位置と速度 | `double X[6]` (x, y, z, vx, vy, vz) |


### 4.2 TRotatedBoundingBox

`TRotatedBoundingBox` は、任意の姿勢（回転）を持つ3次元境界ボックス（Oriented Bounding Box: OBB）を定義し、点の内外判定等の幾何計算を提供するテンプレート構造体です。

#### 4.2.1. テンプレート引数

- `t_value`: 数値型（デフォルトは `double`）
- `using_minmax`: 判定モードの切り替え（`false`: 中心からのサイズ指定、`true`: ローカル座標でのMin/Max指定）

#### 4.2.2. 公開メンバ変数
本構造体は固定長配列をメンバに持ちます。外部から直接操作する際は、要素数への注意が必要です。

##### **t_value X[7]**
- **内容**: ボックスの中心姿勢 $[x, y, z, qx, qy, qz, qw]$。
- **リスク**: 7要素未満の配列として扱うとバッファオーバーランが発生します。

##### **t_value Size[3]**
- **内容**: ボックスの各軸サイズ $[x_{size}, y_{size}, z_{size}]$。
- **使用条件**: `using_minmax` が `false` の場合にのみ判定に使用されます。

##### **t_value Min[3], Max[3]**
- **内容**: ローカル座標系における各軸の最小値および最大値。
- **使用条件**: `using_minmax` が `true` の場合にのみ判定に使用されます。

#### 4.2.3. 公開関数リファレンス

##### **TRotatedBoundingBox (コンストラクタ)**
- **内容**: 構造体のインスタンスを生成し、内部で `Init()` を呼び出して初期化します。

##### **Init**
- **引数**: なし
- **内容**: 姿勢 `X` を単位姿勢（位置0、回転なし $[0,0,0,1]$）に、`Size` をすべて 0.0 に初期化します。
- **リスク**: `Min` および `Max` 配列は**初期化の対象外**です。`using_minmax=true` モードで使用する場合は、別途手動での初期化が必須です。

##### **IsIn**
- **引数**: `const t_value p[3], Eigen::Affine3d *inv_x=NULL`
- **内容**: 指定された点 `p` が境界ボックスの内部（境界線上を含む）に含まれるか判定します。
- **内部処理**:
  1. 点 `p` をボックスのローカル座標系へ変換します。
  2. モードに応じ、`Min/Max` または `Size`（中心から $\pm 0.5 \times Size$）を用いて範囲チェックを行います。
- **計算負荷のリスク**: 
  - 第2引数 `inv_x` が `NULL` の場合、呼び出しのたびに内部で `XToEigMat(X).inverse()`（逆行列計算）が実行されます。大量の点に対して判定を行う場合、パフォーマンスが極端に低下します。
- **メモリ・数値リスク**:
  - 引数 `p` は必ず 3 要素以上のメモリ領域が必要です。
  - 姿勢 `X` のクォータニオンが正規化されていない場合、逆行列計算が破綻し、判定結果が不定（NaNの影響など）になる恐れがあります。

##### **InvX**
- **引数**: なし
- **戻り値**: `Eigen::Affine3d`
- **内容**: 現在の姿勢 `X` の逆変換行列を計算して返します。
- **推奨される運用**: 大量の点に対して `IsIn` を呼び出す際は、事前に本関数で逆行列を取得し、そのポインタを `IsIn` の第2引数へ渡すことで計算コストを最適化してください。

---

#### 4.2.4. 安全管理・使用上の注意
1. **モードの不一致**: `using_minmax` のテンプレート引数と、実際に値をセットした変数（`Size` か `Min/Max` か）が一致していない場合、意図しない判定結果（または未初期化値による判定）となります。
2. **NaNの伝播**: 姿勢 `X` に不正な値が含まれる場合、`IsIn` の結果は信頼できません。
3. **境界値の扱い**: 境界ボックスの表面にちょうど位置する点は「内部（true）」として判定されます。
4. **依存関係**: 本構造体は `XToEigMat` および `EigMatToP` が定義されている環境（`geom_util.h`）でのみ動作します。

---

## 5. 関数リファレンス

### 5.1 変換・抽出 (Eigen ⇔ 配列)
※クォータニオン配列の順序はすべて `[qx, qy, qz, qw]` です。

#### **XToEigMat**
- **引数**: `const t_value x[7]`
- **内容**: 7要素のPose配列 $[x, y, z, qx, qy, qz, qw]$ から `Eigen::Affine3d`（変換行列）を生成します。
- **リスク**: `x` が7要素未満の場合、バッファオーバーランが発生します。クォータニオンの順序が異なる場合、不正な回転行列が生成されます。

#### **QToEigMat**
- **引数**: `const t_value x[4]`
- **内容**: 4要素のクォータニオン配列 $[qx, qy, qz, qw]$ から `Eigen::Quaterniond` を生成します。
- **リスク**: `x` が4要素未満の場合、メモリ不正アクセスが発生します。

#### **EigMatToP**
- **引数**: `const Eigen::Affine3d T, t_value x[3]`
- **内容**: 変換行列 `T` から並進成分を抽出し、3要素配列 `x` に格納します。
- **リスク**: 出力先 `x` に3要素分の書き込み権限が必要です。

#### **EigMatToQ**
- **引数**: `const Eigen::Quaterniond q (または Affine3d T), t_value x[4]`
- **内容**: 回転成分を抽出し、4要素配列 `x` に $[qx, qy, qz, qw]$ の順で格納します。
- **リスク**: 出力先 `x` に4要素分の書き込み権限が必要です。

#### **EigMatToX**
- **引数**: `const Eigen::Affine3d T, t_value x[7]`
- **内容**: 変換行列 `T` を7要素のPose配列 $[x, y, z, qx, qy, qz, qw]$ として一括で書き出します。
- **リスク**: 出力先 `x` に7要素分のメモリ領域が必須です。

### 5.2 座標変換・回転演算

#### **TransformX**
- **引数**: `const t_value x2[7], const t_value x1[7], t_value xout[7]`
- **内容**: 2つのPoseの合成（$x_{out} = x_2 \cdot x_1$）を計算します。
- **リスク**: 全ての引数において7要素の有効なメモリ領域が必要です。

#### **TransformP**
- **引数**: `const t_value x2[7], const t_value p1[3], t_value xout[3]`
- **内容**: 点 $p_1$ をPose $x_2$ によって座標変換した結果を $x_{out}$ に格納します。
- **リスク**: `x2` は7要素、`p1` と `xout` は3要素である必要があります。

#### **RotateAngleAxis**
- **引数**: `const t_value &angle, const t_value axis[3], const t_value q_in[4], t_value q_out[4]`
- **内容**: 姿勢 $q_{in}$ に対し、任意の軸 `axis` と角度 `angle` による回転を加え、結果を $q_{out}$ に格納します。
- **リスク**: `axis` は3要素、`q_in` と `q_out` は4要素必須です。

### 5.3 ベクトル・幾何生成

#### **GetAxisAngle**
- **引数**: `const t_value v1[3], const t_value v2[3], t_array &axis_angle`
- **内容**: $v_1$ を $v_2$ に向けるための回転ベクトル（軸 $\times$ 角度）を算出します。
- **リスク**: $v_1$ と $v_2$ の外積がほぼ 0（平行または逆並行）の場合、計算を打ち切り `axis_angle` をすべて 0 にします。

#### **Orthogonalize**
- **引数**: `vec[3], base[3], out[3], bool original_norm=true`
- **内容**: `base` に直交するように `vec` を修正します。
- **original_norm=false 時**: 出力 `out` は**長さ 1 に正規化**されます。

#### **GetOrthogonalAxisOf**
- **引数**: `axis[3], out[3], preferable[3], fault[]`
- **重大なリスク**: 
    - `axis` と `preferable` が平行な場合（特異点）、`fault == NULL` だと計算が破綻し `NaN` が出力されるかクラッシュします。
    - 特異点時に `fault != NULL` であっても、`fault` が 3 要素未満の配列であればメモリ不正アクセスが発生します。
    - **対策**: 必ず有効な 3 要素配列を `fault` に指定してください。

#### **XFromP1P2**
- **引数**: `p1[3], p2[3], x_out[7], char ax='z', const t_value &r=0.5`
- **内容**: 点 $p_1$ から $p_2$ へ向かう方向を特定の軸 `ax` とするPoseを生成します。
- **リスク**: `p1` と `p2` が同一地点の場合、方向が定義できず、内部でゼロ除算が発生します。

### 5.4 ROS (geometry_msgs) 連携
ROSメッセージと配列形式の相互変換を行います。

#### **PToGPoint / PToGPointVector**
- **引数**: `const t_array p, t_point &point` (Vector版は `ps, points`)
- **内容**: 配列形式の座標を `geometry_msgs/Point` 型にコピーします。
- **リスク**: 入力配列 `p` が **3要素未満** の場合、メモリ不正アクセスによりクラッシュします。

#### **GPointToP**
- **引数**: `const t_point &point, t_array p`
- **内容**: `geometry_msgs/Point` の値を配列 `p` へ書き込みます。
- **リスク**: 出力先 `p` に3要素分の書き込み権限が必要です。

#### **XToGPose / XToGPoseVector**
- **引数**: `const t_array x, t_pose &pose` (Vector版は `xs, poses`)
- **内容**: 7要素のPose配列を `geometry_msgs/Pose` 型へ変換します。
- **リスク**: 配列 `x` は **7要素必須** です。クォータニオンの順序 $[qx, qy, qz, qw]$ を誤ると不正な姿勢になります。

#### **GPoseToX**
- **引数**: `const t_pose &pose, t_array x`
- **内容**: `geometry_msgs/Pose` の値を7要素配列 `x` へ書き込みます。
- **リスク**: 出力先 `x` に7要素分のメモリ領域が必要です。

#### **GenGPoint / GenGQuaternion / GenGRBGA**
- **引数**: 各メンバの初期値
- **内容**: `geometry_msgs` の Point, Quaternion, ColorRGBA インスタンスを生成・初期化して返します。


### 5.5 高度な幾何解析

#### **LeastSq (最小二乗法)**
`Eigen::MatrixXd LeastSq(const Eigen::MatrixXd &X, const Eigen::MatrixXd &Y, const double &lambda=0.01)`
- **リスク**: `X.transpose()*X` が正則でない（ランク落ち）場合、`.inverse()` で計算が破綻します。
- **対策**: 正則化パラメータ `lambda` を適切に設定することで回避しますが、極端に小さい値を設定すると数値的に不安定になります。

#### **GetVisualNormal (法線可視化)**
`void GetVisualNormal(const t_1 &nx, ny, nz, t_2 &r, g, b)`
- **ロジック**: 法線の向きが反転しても同じ色にならないよう、`nz < 0` の場合に反転処理を行っています（Version 2）。
- **注意**: `nx, ny, nz` は正規化されている（長さ1）ことを前提としています。未正規化だと色空間 [0, 1] を逸脱します。

---

## 6. 特記事項
- **Eigenの仕様**: `Eigen::Quaterniond` の引数順序 `(w, x, y, z)` と、本ライブラリ配列の `[x, y, z, w]` 順序の変換は内部で適切にラップされています。
- **パフォーマンス**: すべての関数は `inline` 定義されており、ランタイムのオーバーヘッドは最小限です。