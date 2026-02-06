# ROS2 移行メモ

## はじめに

このパッケージにはノードがない。しかしROSの機能は使っている

含まれるもの
1. ROS非依存ヘッダ cpp_util とか
1. ROS非依存ライブラリ optimizer
1. ROS依存ヘッダ rviz_util

## 手順

### 環境構築のための docker 設定

Dockerfile
```docker
# Default arguments
ARG USER_NAME=fv
ARG USER_UID=1000
ARG USER_GID=1000

# Stage: Final Image
FROM osrf/ros:humble-desktop
ARG USER_NAME
ARG USER_UID
ARG USER_GID

USER root

# 必要なパッケージ（sudo等）のインストール
RUN apt-get update && apt-get install -y sudo && rm -rf /var/lib/apt/lists/*

# ユーザーの作成とホームディレクトリの固定
RUN groupadd --gid $USER_GID $USER_NAME || true \
    && useradd --uid $USER_UID --gid $USER_GID -m -s /bin/bash $USER_NAME || true \
    && echo "$USER_NAME ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# 環境変数の設定
ENV HOME=/home/${USER_NAME}
ENV ROS_DISTRO=humble

# ユーザーを切り替えて作業開始
USER ${USER_NAME}
WORKDIR ${HOME}

# 必要なフォルダを一括作成
RUN mkdir -p \
    ${HOME}/workspace/general_ws \
    ${HOME}/workspace/ros_ws \
    ${HOME}/workspace/external_general_ws \
    ${HOME}/workspace/external_ros_ws

# .bashrc に source 設定を追記
# (local_setup.bash はファイルが存在しないとエラーを出す場合があるため、
#  [ -f ... ] でチェックを入れる)
RUN echo "source /opt/ros/\$ROS_DISTRO/setup.bash" >> ${HOME}/bashrc_append \
    && echo "if [ -f ${HOME}/workspace/external_ros_ws/local_setup.bash ]; then source ${HOME}/workspace/external_ros_ws/local_setup.bash; fi" >> ${HOME}/bashrc_append \
    && echo "if [ -f ${HOME}/workspace/ros_ws/local_setup.bash ]; then source ${HOME}/workspace/ros_ws/local_setup.bash; fi" >> ${HOME}/bashrc_append \
    && cat ${HOME}/bashrc_append >> ${HOME}/.bashrc \
    && rm ${HOME}/bashrc_append

# コンテナ起動時のカレントディレクトリ
WORKDIR ${HOME}
```

起動

```bash
# イメージ生成
sudo docker build \
  --build-arg USER_NAME=$(whoami) \
  --build-arg USER_UID=$(id -u) \
  --build-arg USER_GID=$(id -g) \
  --tag ros2:humble_fv .

# コンテナ起動 (SSH鍵共有，ホストのグラフィック環境使用，共有フォルダ付き)
sudo docker container run -it \
  --name ros2_humble_x11_fv \
  --net host \
  --privileged \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -e SSH_AUTH_SOCK=/ssh-agent \
  -v $SSH_AUTH_SOCK:/ssh-agent \
  -v /home/$(whoami)/docker/shared_ws:/home/$(whoami)/shared_ws \
  ros2:humble_fv \
  bash
```

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

### rviz_util の ROS2 対応およびテスト

RViz を起動する必要があるので，launchテストにする必要がある。  
このテストは unittest で起動する。py_test で実行する方法はわからず，現状は諦め状態

1. launchファイル test_rviz_util.launch.py を追加  
   rvizとテストを起動し，C++のテストが終わるのを待つ設定
1. RViz設定ファイル test.rviz を追加  
   あらかじめトピック受信用のオブジェクトを登録している
1. rviz_util.h を ROS2 向けに修正
1. test_rviz_util.cpp を追加して簡単なテストを作成 
   ノードを起こして rviz_util の処理を呼ぶ。正しくマーカが登録されたかどうかを調べる処理は入れていない
1. Pythonにパスを通すために setting.json を修正
1. package.xml に launch テスト用の依存パッケージを登録
1. CMakeLists.txt に launch テストの登録処理を追加
1. テストが実行できることを確認
   ```bash
   colcon test --packages-select ay_cpp  --event-handlers console_direct+ --ctest-args -R test_rviz_util.launch.py
   ```

詳細な変更点はコミット 1c30dd9a14b896327dbfae6eb49e1ea0e1e6ec1d を確認のこと

## Notes

launch テストを単体で実行したいときは launch_test も使える
```bash
launch_test install/ay_cpp/share/ay_cpp/test/launch/test_rviz_util.launch.py
```

`colcon test` でリンタを動かさない設定
```bash
colcon test --packages-select ay_cpp  --event-handlers console_direct+ --ctest-args -E "(copyright|lint|flake8|cppcheck|uncrustify)"
```

CIなどでRVizを起動したくない場合，仮想ディスプレイを使う
```bash
sudo apt update
sudo apt install xvfb
xvfb-run -a colcon test --packages-select ay_cpp  --event-handlers console_direct+ --ctest-args -R test_rviz_util.launch.py
```
