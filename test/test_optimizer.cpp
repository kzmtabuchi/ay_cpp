#include <gtest/gtest.h>
#include <vector>
#include <rclcpp/rclcpp.hpp>
#include "ay_cpp/optimizer.h"

TEST(OptimizerTest, CallTest)
{
  // 使い方がよくわからんので，とりあえず関数呼ぶだけ
  trick::TCMAESParams params;

  double x0[1] = {0.0};
  double sig0[1] = {1.0};
  double xmin[1] = {-5.0};
  double xmax[1] = {5.0};
  double res = 0.0;
  trick::MinimizeF(
    [](const double x[], bool & is_feasible) -> double
    {
      is_feasible = true;
      return x[0] * x[0];  // Simple quadratic function
    },
    /*x0=*/ x0, /*sig0=*/ sig0, /*dim=*/ 1,
    /*xmin=*/ xmin, /*xmax=*/ xmax, /*bound_len=*/ 1,
    /*xres=*/ &res,
    params
  );
}
