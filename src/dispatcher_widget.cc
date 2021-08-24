#include "dispatcher/dispatcher_widget.h"
#include "dispatcher/dispatch_item.h"
#include "dispatcher/dispatcher_node.h"

#include "dispatcher/config.h"

#include <stdio.h>

#include <iostream>
#include <string>

#include <QCoreApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QSocketNotifier>
#include <QSpacerItem>
#include <QWidget>

#include <rclcpp/node_interfaces/node_base_interface.hpp>

#include <yaml-cpp/yaml.h>

#include <cfw/cfw.h>

std::string dispatcher::version()
{
  std::string version_major = std::to_string(DISPATCHER_VERSION_MAJOR);
  std::string version_minor = std::to_string(DISPATCHER_VERSION_MINOR);
  std::string version_patch = std::to_string(DISPATCHER_VERSION_PATCH);
  return version_major + std::string(".") + version_minor + std::string(".") +
         version_patch;
}

std::string dispatcher::branch() { return std::string(DISPATCHER_GIT_BRANCH); }

std::string dispatcher::commit() { return std::string(DISPATCHER_GIT_COMMIT); }

static bool check_tmux_exists()
{
  return (system("which tmux") == 0) ? true : false;
}

static void check_tmux_exists_throw_exception()
{
  if (!check_tmux_exists()) {
    throw dispatcher::DispatcherException(
        "Program `tmux` not found on this system, terminating...");
  }
}

void dispatcher::DispatcherWidget::EnableScripts(bool enable)
{
  if (script_group_box_ == nullptr) {
    EVR_FATAL_PTR(ros_node_, "script_group_box_ was not correctly initialized");
  }
  script_group_box_->setVisible(enable);
}

/*!
@brief class constructor for DispatcherWidget application
*/
dispatcher::DispatcherWidget::DispatcherWidget(QWidget* parent)
    : QWidget(parent)
{
  check_tmux_exists_throw_exception();

  QVBoxLayout* layout_ = new QVBoxLayout(parent);

  // upper group box / grid
  QGroupBox* upper_box = new QGroupBox();
  upper_box->setStyleSheet(QString("QGroupBox {border:0}"));
  upper_box->setContentsMargins(QMargins(-1, -1, -1, 0));
  grid_layout_ = new QGridLayout(upper_box);
  layout_->addWidget(upper_box);

  // lower group box / grid
  script_group_box_ = new QGroupBox();
  script_group_box_->setStyleSheet(QString("QGroupBox {border:0}"));
  script_group_box_->setContentsMargins(QMargins(-1, -1, -1, 0));
  script_layout_ = new QGridLayout(script_group_box_);
  layout_->addWidget(script_group_box_);

  ros_node_ = std::make_shared<dispatcher::DispatcherNode>(this);
  
  int          index = grid_layout_->rowCount();
  QPushButton* start = new QPushButton("start all checked", this);
  start->setStyleSheet(QString("color: green"));
  start->setFlat(false);
  grid_layout_->addWidget(start, index, 2);
  connect(start, SIGNAL(clicked()), this, SLOT(StartAllCheckedCb()));

  QPushButton* stop = new QPushButton("stop all checked", this);
  stop->setStyleSheet(QString("color: red"));
  grid_layout_->addWidget(stop, index, 3);
  connect(stop, SIGNAL(clicked()), this, SLOT(StopAllCheckedCb()));

  QSpacerItem* spacer =
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  grid_layout_->addItem(spacer, index, 4);

  QSpacerItem* spacer2 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  grid_layout_->addItem(spacer2, grid_layout_->rowCount(), 0);

  double loop_period_ms = 1.0 / ros_node_->get_target_loop_rate_hz() * 1000.0;
  EVR_DIAGNOSTIC_PTR(
    ros_node_, "Using loop period %f ms", loop_period_ms);

  timer_ = new QTimer;
  connect(timer_, SIGNAL(timeout()), this, SLOT(Process()));
  timer_->start(loop_period_ms);

  layout_->setSizeConstraint(QLayout::SetFixedSize);
  layout_->setSpacing(0);
  setLayout(layout_);
}

/*!
@brief class destructor
*/
dispatcher::DispatcherWidget::~DispatcherWidget() {}

void dispatcher::DispatcherWidget::Process()
{
  if (rclcpp::ok()) {
    ros_node_->Process();
    ros_executor_.spin_node_once(ros_node_);
  } else {
    EVR_ACTIVITY_HI_PTR(ros_node_, "Shutting down");
    ros_node_->StopAll();
    rclcpp::shutdown();
    QCoreApplication::quit();
  }
}

void dispatcher::DispatcherWidget::StartAllCheckedCb()
{
  ros_node_->StartChecked();
}

void dispatcher::DispatcherWidget::StopAllCheckedCb()
{
  ros_node_->StopChecked();
}

void dispatcher::DispatcherWidget::closeEvent(QCloseEvent*)
{
  EVR_ACTIVITY_HI_PTR(ros_node_, "Shutting down");
  ros_node_->StopAll();
  rclcpp::shutdown();
  QCoreApplication::quit();
}
