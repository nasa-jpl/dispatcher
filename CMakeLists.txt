cmake_minimum_required(VERSION 3.11)
project(dispatcher
  DESCRIPTION "Casah node dispatcher"
  VERSION 0.0.30
  LANGUAGES C CXX
)
find_package(CfwCmake)

cfw_configure_project()
cfw_get_project_default_options()
cfw_install_project_headers(src)
cfw_add_target_format(src)
cfw_add_target_doc()
cfw_add_target_tag()

option(BUILD "Optionally disable building package by setting BUILD=OFF" ON)
if(BUILD)

  cfw_fetch_dependencies(
    casah_node git@fornat1.jpl.nasa.gov:casah/ros2/libs/casah_node.git v0.3.21
  )

  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTORCC ON)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)

  find_package(ament_cmake REQUIRED)
  find_package(rclcpp REQUIRED)
  find_package(std_msgs REQUIRED)
  find_package(sensor_msgs REQUIRED)
  find_package(casah_msgs REQUIRED)

  find_package(builtin_interfaces REQUIRED)
  find_package(rosidl_default_generators REQUIRED)
  find_package(std_srvs REQUIRED)
  find_package(
    Qt5
    COMPONENTS
    Core
    Widgets
    OpenGL
    Gui
    REQUIRED
  )
  find_package(yaml_cpp_vendor REQUIRED)

  if(BUILD_TESTING)
    find_package(ament_lint_auto REQUIRED)
    ament_lint_auto_find_test_dependencies()
  endif()

  add_library(dispatcher_widget
    src/dispatcher_widget.cc
    src/dispatcher_widget.h
    src/dispatcher_item.cc
    src/dispatcher_item.h
    src/dispatcher_node.cc
    src/dispatcher_node.h
    src/script_item.cc
    src/script_item.h
  )

  target_link_libraries(
    dispatcher_widget
    Qt5::Core
    Qt5::Widgets
    Qt5::OpenGL
    Qt5::Gui
  )

  add_library(icons STATIC icons/icons.qrc)

  target_compile_options(dispatcher_widget PUBLIC
    -Wall
    -D_GNU_SOURCE=1
    -pipe
    -fPIC
    -Wextra
    -march=x86-64
    -mtune=generic
    -fstack-protector-strong
    -D_REENTRANT
    -DQT_WIDGETS_LIB
    -DQT_GUI_LIB
    -DQT_CORE_LIB
    -lstdc++11
    -std=c++11
    -Wall
    -Wextra
    -ggdb3
  )

  target_include_directories(dispatcher_widget
    PUBLIC
    ${CMAKE_BINARY_DIR}/include
  )

  target_link_libraries(
    dispatcher_widget
    casah_node
    icons
  )

  ament_target_dependencies(dispatcher_widget
    rclcpp
    std_srvs
    sensor_msgs
    casah_msgs
    yaml_cpp_vendor
  )

  add_executable(
    dispatcher
    src/dispatcher.cc
  )

  target_link_libraries(
    dispatcher
    stdc++
    dispatcher_widget
  )

  target_compile_options(
    dispatcher PUBLIC
    -ggdb3
  )

  install(TARGETS
    dispatcher
    DESTINATION lib/${PROJECT_NAME}
  )

  ament_package()

endif()
