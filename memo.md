# ROS2 移行メモ

## はじめに

このパッケージにはノードがない。しかしROSの機能は使っている

含まれるもの
1. ROS非依存ヘッダ cpp_util とか
1. ROS非依存ライブラリ optimizer
1. ROS依存ヘッダ rviz_util

## 手順

### とりあえずビルドできるようにするまで

1. フォークをきってブランチを作成 https://github.com/kzmtabuchi/ay_cpp/tree/devel_ros2_humble
1. ダミーでからパッケージ作成
1. ダミーのフォルダ名を適当に変えて，作成したブランチをチェックアウト
   ```bash
   cd ~/workspace/ros_ws/src
   git checkout -b devel_ros2_humble git@github.com:kzmtabuchi/ay_cpp.git
   ```
1. チェックアウトしたパッケージに含まれる CMakeLists.txt, package.xml を適当な名称に変更
1. ダミーのパッケージから CMakeLists.txt, package.xml を持ってくる
1. 適当な名前に変えたチェックアウトしたパッケージに含まれる CMakeLists.txt, package.xml を[ros2_template_cpp](https://github.com/kzmtabuchi/ros2_template_cpp) を参照しながらダミーのパッケージから持ってきた CMakeLists.txt, package.xml へマージ
1. ros2_template_cpp から VSCode向け設定を持ちこみ
1. .gitignore へ build などがちゃんと無視されるように追記
1. ビルド → OK
   ```bash
   cd ~/workspace/ros_ws
   colcon build --packages-select ay_cpp
   ```

### cpp_util, optimizer のテスト

1. test ディレクトリにテスト用cppを追加
1. CMakeLists.txt にテストターゲットを追加
1. テスト用cppを追加して簡単なテストを記載
1. フォーマット調整
   ```bash
   ament_uncrustify --reformat /home/fv/workspace/ros_ws/src/ay_cpp/test
   ```
1. 追加したテストを実行 → OK
   ```bash
   colcon test --packages-select ay_cpp --ctest-args -R test_ay_cpp
   ```
1. 全体のテストを実行 → 失敗, 既存コードの規約違反が大量発生
   ```bash
   colcon test --packages-select ay_cpp
   ```

詳細な変更点はコミット f89f56af55fcb54f0be45c46a23ef9ab3d25251f を確認のこと

### rviz_util のテスト

RViz を起動する必要があるので，launchテストにする必要がある。  
このテストは unittest で起動する。py_test で実行する方法はわからず，現状は諦め状態

1. launchファイル test_rviz_util.launch.py を追加  
   rvizとテストを起動し，C++のテストが終わるのを待つ設定
1. RViz設定ファイル test.rviz を追加  
   あらかじめトピック受信用のオブジェクトを登録している
1. rviz_util.h を ROS2 向けに修正
1. テストコード test_rviz_util.cpp の追加  
   ノードを起こして rviz_util の処理を呼ぶ。正しくマーカが登録されたかどうかを調べる処理は入れていない
1. Pythonにパスを通すために setting.json を修正
1. package.xml に launch テスト用の依存パッケージを登録
1. CMakeLists.txt に launch テストの登録処理を追加

詳細な変更点はコミット 1c30dd9a14b896327dbfae6eb49e1ea0e1e6ec1d を確認のこと