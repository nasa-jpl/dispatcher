#include "dispatcher/dispatcher_widget.h"
#include "dispatcher/dispatcher_node.h"
#include "dispatcher/dispatch_item.h"

#include "dispatcher/config.h"

#include <stdio.h>

#include <string>
#include <iostream>

#include <QWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QSpacerItem>
#include <QSocketNotifier>
#include <QCoreApplication>
#include <QPushButton>

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


static bool check_tmux_exists() {
  return (system("which tmux") == 0) ? true : false;
}


static void check_tmux_exists_throw_exception() {
  if(!check_tmux_exists()) {
    throw dispatcher::DispatcherException(
      "Program `tmux` not found on this system, terminating..."); 
  }
}

/*!
@brief class constructor for DispatcherWidget application
*/
dispatcher::DispatcherWidget::DispatcherWidget(QWidget* parent)
  : QWidget(parent) {

  check_tmux_exists_throw_exception();

  QGroupBox* main_box = new QGroupBox;
  main_box->setContentsMargins(QMargins(0, 0, 0, 0));

  layout_ = new QGridLayout(main_box);
  setLayout(layout_);

  ros_node_ = std::make_shared<dispatcher::DispatcherNode>(this);
  
  int index = layout_->rowCount();
  QPushButton *start = new QPushButton("start all checked", this);
  start->setStyleSheet(QString("color: green"));
  layout_->addWidget(start, index, 2);
  connect(start, SIGNAL(clicked()), this, SLOT(StartAllCheckedCb()));

  QPushButton *stop = new QPushButton("stop all checked", this);
  stop->setStyleSheet(QString("color: red"));
  layout_->addWidget(stop, index, 3);
  connect(stop, SIGNAL(clicked()), this, SLOT(StopAllCheckedCb()));

  QSpacerItem *spacer =
    new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  layout_->addItem(spacer, index, 4);

  QSpacerItem* spacer2 = 
    new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  layout_->addItem(spacer2, layout_->rowCount(), 0);


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
    ros_executor_.spin_node_once(ros_node_);
  } else {
    CFW_INFO("SHUTTING DOWN");
    ros_node_->StopAll();
    rclcpp::shutdown();
    QCoreApplication::quit();
  }
}


void dispatcher::DispatcherWidget::StartAllCheckedCb() {
  ros_node_->StartChecked();
}


void dispatcher::DispatcherWidget::StopAllCheckedCb() {
  ros_node_->StopChecked();
}

