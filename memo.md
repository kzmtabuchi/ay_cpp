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

### rviz_util のテスト

まだ実施していない。テスト用にノードとか起こす必要あり
