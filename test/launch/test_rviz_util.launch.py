import os
import unittest
import launch_testing
import launch_testing.actions
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def get_launch_description():
    pkg_path = get_package_share_directory('ay_cpp')
    rviz_config_path = os.path.join(pkg_path, "test", "rviz", "test.rviz")

    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        arguments=['-d', rviz_config_path],
        output='screen'
    )
    gtest_node = Node(
        package='ay_cpp',
        executable='test_ay_cpp_rviz',
        output='screen'
    )

    return LaunchDescription([
        rviz_node,
        gtest_node,
        launch_testing.actions.ReadyToTest(),
    ]), gtest_node


def generate_launch_description():
    # ros2 launch が呼ぶ関数

    ld, _ = get_launch_description()
    return ld


def generate_test_description():
    # colcon test (launch_testing) が呼ぶ関数

    ld, gtest = get_launch_description()
    return ld, {'gtest': gtest}


class TestGtestBasic(unittest.TestCase):
    # unittest 形式のテストクラス
    def test_gtest_run(self, proc_info, gtest):
        # C++側の終了まで最大60秒待機
        proc_info.assertWaitForShutdown(process=gtest, timeout=60)


@launch_testing.post_shutdown_test()
class TestShutdown(unittest.TestCase):
    # テスト終了後に残っているプロセスを終了させる
    def test_exit_codes(self, proc_info):
        launch_testing.asserts.assertExitCodes(proc_info)
