#include "dispatcher/detail/logic.h"
#include "dispatcher/ros_process_item.h"
#include "dispatcher/dispatcher_widget.h"

#include <stdlib.h>

#include <algorithm>
#include <regex>
#include <string>

#include <QHBoxLayout>
#include <QIcon>
#include <QPixmap>
#include <QString>
#include <QVBoxLayout>

#include <rclcpp/rclcpp.hpp>

/*!
@brief class constructor for RosProcessItem
*/
dispatcher::RosProcessItem::RosProcessItem(QWidget*                    parent,
                                           dispatcher::DispatcherNode* ros_node,
                                           const YAML::Node&           node,
                                           QGridLayout*                layout)
    : dispatcher::ProcessItem(parent, ros_node, node, layout)
{
  // Expecting process_items to have `namespace`, `node_name` and `ros_nodes` to
  // have keys, so handle them in this class
  if (node["namespace"] && node["node_name"]) {
    RosNodeMonitorConfig monitor_config;
    monitor_config.namespace_ = node["namespace"].as<std::string>();
    monitor_config.name       = node["node_name"].as<std::string>();
    ros_nodes_.push_back(monitor_config);
  }
  if (node["ros_nodes"]) {
    for (const auto& ros_node : node["ros_nodes"]) {
      RosNodeMonitorConfig monitor_config;
      monitor_config.namespace_ = ros_node["namespace"].as<std::string>();
      monitor_config.name       = ros_node["name"].as<std::string>();
      ros_nodes_.push_back(monitor_config);
    }
  }
}

/*!
@brief class destructor
*/
dispatcher::RosProcessItem::~RosProcessItem() {}

void dispatcher::RosProcessItem::Process()
{
  if (!current_configuration_) {
    return;
  }

  if (!enabled_) {
    return;
  }

  const auto status = dispatcher::detail::SummarizeRosStatus(
      ros_nodes_, current_configuration_->ros_nodes,
      ros_node_->get_online_nodes());
  online_ = status.online;
  if (status.found != num_online_nodes_prev_) {
    RCLCPP_INFO(ros_node_->get_logger(),
                "Status change for node %s, %ld/%ld nodes online",
                name_.c_str(), status.found, status.expected);
    num_online_nodes_prev_ = status.found;
    label_->setToolTip(status.tooltip.c_str());
    if (status.color == dispatcher::detail::StatusColor::kRed) {
      label_->setPixmap(red_status_icon_);
      label_->setStyleSheet(QString("color: red"));
    } else if (status.color == dispatcher::detail::StatusColor::kOrange) {
      label_->setPixmap(orange_status_icon_);
      label_->setStyleSheet(QString("color: orange"));
    } else {
      label_->setPixmap(green_status_icon_);
      label_->setStyleSheet(QString("color: green"));
    }
  }
}

void dispatcher::RosProcessItem::StartCb()
{
  if (!dispatcher::ProcessItem::PrepareTmuxSession()) {
    return;
  }

  // cd to workspace directory and source ros environment
  TmuxSendKeys("C-U");  // Clears the line of any text
  TmuxSendKeys("cd " + ros_node_->get_workspace());
  TmuxSendKeys("source install/setup.bash");

  dispatcher::ProcessItem::StartCb();
}

void dispatcher::RosProcessItem::StopCb() { dispatcher::ProcessItem::StopCb(); }

void dispatcher::RosProcessItem::TerminalCb()
{
  dispatcher::ProcessItem::TerminalCb();
}
