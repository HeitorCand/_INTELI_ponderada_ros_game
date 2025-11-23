
cmake_minimum_required(VERSION 3.8)
project(cg_autonomous)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cg_interfaces REQUIRED)
find_package(std_msgs REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Development)  # Add this

# Include directories
include_directories(include)

# Pathfinding library
add_library(pathfinding src/pathfinding.cpp)
ament_target_dependencies(pathfinding rclcpp)

# Autonomous navigator node
add_executable(autonomous_navigator src/autonomous_navigator.cpp)
target_link_libraries(autonomous_navigator pathfinding Python3::Python)  # Add Python3::Python here
ament_target_dependencies(autonomous_navigator 
  rclcpp 
  cg_interfaces
  std_msgs
)

if(APPLE)
  set_target_properties(pathfinding PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
  set_target_properties(autonomous_navigator PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
endif()

# Install targets
install(TARGETS
  autonomous_navigator
  DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS
  pathfinding
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY
  include/
  DESTINATION include
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
