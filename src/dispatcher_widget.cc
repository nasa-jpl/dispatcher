#include "dispatcher/dispatcher_widget.h"
#include "dispatcher/dispatcher_node.h"
#include "dispatcher/dispatch_item.h"

#include "dispatcher/config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <iostream>

#include <QWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QSpacerItem>
#include <QSocketNotifier>
#include <QCoreApplication>

#include <rclcpp/node_interfaces/node_base_interface.hpp>

#include <yaml-cpp/yaml.h>

#include <cfw/cfw.h>

std::string dispatcher::version() {
  std::string version_major = std::to_string(DISPATCHER_VERSION_MAJOR);
  std::string version_minor = std::to_string(DISPATCHER_VERSION_MINOR);
  std::string version_patch = std::to_string(DISPATCHER_VERSION_PATCH);
  return version_major + std::string(".") + version_minor + std::string(".") +
         version_patch;
}

std::string dispatcher::branch() {
  return std::string(DISPATCHER_GIT_BRANCH);
}

std::string dispatcher::commit() {
  return std::string(DISPATCHER_GIT_COMMIT);
}


/*!
@brief class constructor for DispatcherWidget application
*/
dispatcher::DispatcherWidget::DispatcherWidget(QWidget* parent)
  : QWidget(parent) {

  QGroupBox* main_box = new QGroupBox;
  main_box->setContentsMargins(QMargins(0, 0, 0, 0));

  layout_ = new QGridLayout(main_box);
  setLayout(layout_);

  QSpacerItem* spacer = 
    new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  layout_->addItem(spacer, layout_->rowCount(), 0);

  ros_node_ = std::make_shared<dispatcher::DispatcherNode>(this);

  double loop_period_ms = 1.0 / ros_node_->get_target_loop_rate_hz() * 1000.0;
  CFW_INFO("Using loop period %f ms", loop_period_ms);
  
  timer_ = new QTimer;
  connect(timer_, SIGNAL(timeout()), this, SLOT(Process())); 
  timer_->start(loop_period_ms);
}


/*!
@brief class destructor
*/
dispatcher::DispatcherWidget::~DispatcherWidget() {}


void dispatcher::DispatcherWidget::Process() {
  if(rclcpp::ok()) {
    ros_node_->Process();
    // auto online_nodes = ros_node_->get_online_nodes();
    // ros_executor_.spin_node_once(ros_node_);
  } else {
    CFW_INFO("SHUTTING DOWN");
    rclcpp::shutdown();
    QCoreApplication::quit();
  }
}
