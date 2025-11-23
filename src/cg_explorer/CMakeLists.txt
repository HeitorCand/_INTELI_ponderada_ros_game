cmake_minimum_required(VERSION 3.10)
project(cg_explorer)

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

# Libraries
add_library(mapper src/mapper.cpp)
ament_target_dependencies(mapper rclcpp)

add_library(pathfinder src/pathfinder.cpp)
target_link_libraries(pathfinder mapper)
ament_target_dependencies(pathfinder rclcpp)

# Explorer node
add_executable(explorer_node src/explorer_node.cpp)
target_link_libraries(explorer_node mapper pathfinder Python3::Python)
    
ament_target_dependencies(explorer_node 
  rclcpp 
  cg_interfaces
  std_msgs
)

if(APPLE)
  set_target_properties(mapper PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
  set_target_properties(explorer_node PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
endif()

# Install targets
install(TARGETS
  explorer_node
  DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS
  mapper
  pathfinder
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/
  DESTINATION include
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
