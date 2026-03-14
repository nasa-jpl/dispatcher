cmake_minimum_required(VERSION 3.11)
project(dispatcher
  DESCRIPTION "Node dispatcher"
  VERSION 0.1.5
  LANGUAGES CXX
)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets OpenGL Gui)
find_package(yaml-cpp REQUIRED)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(icons STATIC
  icons/icons.qrc
)

add_library(dispatcher_widget
  include/dispatcher/dispatcher_node.h
  include/dispatcher/dispatcher_widget.h
  include/dispatcher/item.h
  include/dispatcher/process_item.h
  include/dispatcher/ros_process_item.h
  include/dispatcher/script_item.h
  include/dispatcher/shell_process_item.h
  include/dispatcher/variable.h
  src/dispatcher_widget.cc
  src/dispatcher_node.cc
  src/item.cc
  src/process_item.cc
  src/script_item.cc
  src/shell_process_item.cc
  src/ros_process_item.cc
  src/variable.cc
)

target_include_directories(dispatcher_widget
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(dispatcher_widget PUBLIC cxx_std_17)

target_compile_options(dispatcher_widget
  PRIVATE
    -D_GNU_SOURCE=1
)

# Debug-only flags
target_compile_options(dispatcher_widget
  PRIVATE
    $<$<CONFIG:Debug>:-ggdb3>
)

ament_target_dependencies(dispatcher_widget
  rclcpp
  sensor_msgs
  std_srvs
)

target_link_libraries(dispatcher_widget
  Qt5::Core
  Qt5::Widgets
  Qt5::OpenGL
  Qt5::Gui
  yaml-cpp
  icons
)

add_executable(dispatcher
  src/dispatcher.cc
)

target_link_libraries(dispatcher
  PRIVATE
    dispatcher_widget
)

target_compile_options(dispatcher
  PRIVATE
    $<$<CONFIG:Debug>:-ggdb3>
)

install(
  TARGETS dispatcher
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
