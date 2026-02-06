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

  bool received = false;
  visualization_msgs::msg::Marker last_msg;

  // 検証用サブスクライバ
  auto qos = rclcpp::QoS(rclcpp::KeepLast(10)).reliable().transient_local();  // RVizデフォルトのQoSに合わせる
  auto sub = node->create_subscription<visualization_msgs::msg::Marker>(
    "visualization_marker", qos,
    [&](const visualization_msgs::msg::Marker::SharedPtr msg) {
      received = true;
      last_msg = *msg;
    });

  // 接続待ち: Subscriberが現れるまで待機
  rclcpp::WallRate wait_rate(10);
  auto start_time = node->now();
  while (node->count_subscribers("visualization_marker") < 2) {
    rclcpp::spin_some(node);
    wait_rate.sleep();

    if ((node->now() - start_time).seconds() > 10.0) {
      FAIL() << "Timeout waiting for visualization_marker subscriber";
    }
  }

  // キューブを表示
  geometry_msgs::msg::Pose pose;
  pose.position.z = 0.5;   // 地面に埋まらないように少し浮かせる
  auto scale = trick::GenGPoint<geometry_msgs::msg::Vector3>(0.5, 0.3, 0.2);
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

  // 内容の検証
  ASSERT_TRUE(received);
  EXPECT_EQ(last_msg.type, visualization_msgs::msg::Marker::CUBE);
  EXPECT_NEAR(last_msg.scale.x, scale.x, 1e-5);
  EXPECT_NEAR(last_msg.scale.y, scale.y, 1e-5);
  EXPECT_NEAR(last_msg.scale.z, scale.z, 1e-5);
  EXPECT_NEAR(last_msg.pose.position.x, pose.position.x, 1e-5);
  EXPECT_NEAR(last_msg.pose.position.y, pose.position.y, 1e-5);
  EXPECT_NEAR(last_msg.pose.position.z, pose.position.z, 1e-5);
  EXPECT_STREQ(last_msg.ns.c_str(), "visualizer");
}
