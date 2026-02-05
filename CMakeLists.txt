cmake_minimum_required(VERSION 3.8)
project(ay_cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 17)

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)  # roscppに対応

# ライブラリターゲットの作成
add_library(ay_cpp
  src/optimizer.cpp
  include/ay_cpp/3rdparty/cma_es/cmaes.c
  include/ay_cpp/3rdparty/cma_es/boundary_transformation.c
)
ament_target_dependencies(ay_cpp rclcpp)  # いらないかも

target_include_directories(ay_cpp PUBLIC
  "$<INSTALL_INTERFACE:include>"  # catkin_INCLUDE_DIRSに対応
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)

# いらないかもしれないが，一応インストールしておく
install(TARGETS
  ay_cpp
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  # テストターゲットの作成
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_ay_cpp test/test_cpp_util.cpp test/test_optimizer.cpp)
  target_include_directories(test_ay_cpp PUBLIC
    "$<INSTALL_INTERFACE:include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_cpp>"
  )
  ament_target_dependencies(test_ay_cpp rclcpp)
  target_link_libraries(test_ay_cpp ay_cpp)

  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()  # catkin_packageに対応
