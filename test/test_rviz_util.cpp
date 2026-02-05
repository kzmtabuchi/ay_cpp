#include <gtest/gtest.h>
#include "ay_cpp/geom_util.h"
#include "ay_cpp/rviz_util.h"

class RvizUtilTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    rclcpp::init(0, nullptr);
    node = std::make_shared<rclcpp::Node>("test_node");
    visualizer.Setup(node);
  }
  void TearDown() override {rclcpp::shutdown();}

  std::shared_ptr<rclcpp::Node> node;
  trick::TSimpleVisualizer visualizer;
};

TEST_F(RvizUtilTest, AddCubeTest) {
  geometry_msgs::msg::Pose pose;
  pose.position.z = 0.5;   // 地面に埋まらないように少し浮かせる

  auto scale = trick::GenGPoint<geometry_msgs::msg::Vector3>(0.5, 0.3, 0.2);

  // 接続待ち: Subscriberが現れるまで待機
  rclcpp::WallRate wait_rate(10);
  auto start_time = node->now();
  while (node->count_subscribers("visualization_marker") == 0) {
    rclcpp::spin_some(node);
    wait_rate.sleep();

    if ((node->now() - start_time).seconds() > 10.0) {
      FAIL() << "Timeout waiting for visualization_marker subscriber";
    }

    RCLCPP_INFO(node->get_logger(), "Waiting for subscribers... count: %ld", 
            node->count_subscribers("/visualization_marker"));
  }

  // キューブを表示
  visualizer.SetDt(rclcpp::Duration::from_seconds(10.0));
  visualizer.AddCube(pose, scale);

  // 少し待機
  int wait_time = 1;   // 目視したいときは待ち時間を伸ばす
  RCLCPP_INFO(node->get_logger(), "Test finished. Holding GUI for %d seconds...", wait_time);
  rclcpp::WallRate loop_rate_keep(1);
  for (int i = 0; i < wait_time; ++i) {
    rclcpp::spin_some(node);
    loop_rate_keep.sleep();
  }
  SUCCEED();
}
