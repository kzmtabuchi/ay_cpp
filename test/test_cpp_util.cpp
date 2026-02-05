#include <gtest/gtest.h>
#include <vector>
#include <rclcpp/rclcpp.hpp>
#include "ay_cpp/cpp_util.h"

TEST(CppUtilTest, EmptyTest)
{
  rclcpp::init(0, nullptr);
  rclcpp::shutdown();
}

TEST(CppUtilTest, SqTest)
{
  EXPECT_EQ(trick::Sq(3), 9);
  EXPECT_NEAR(trick::Sq(1.5), 2.25, 1e-9);
}

TEST(CppUtilTest, MeanTest)
{
  EXPECT_EQ(trick::Mean(std::vector<int>{1, 2, 3}), 2);
  EXPECT_NEAR(trick::Mean(std::vector<double>{1.0, 2.0, 3.0}), 2.0, 1e-9);
}

// 他のテストは省略
