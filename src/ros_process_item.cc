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

  const auto& online_nodes_             = ros_node_->get_online_nodes();
  size_t      num_online_nodes_found    = 0;
  size_t      num_expected_online_nodes = 0;
  std::string online_nodes_str;

  // does current configuration have any ros nodes specified?
  // if yes, then use current configuration:
  if (current_configuration_->ros_nodes.size() > 0) {
    num_expected_online_nodes = current_configuration_->ros_nodes.size();
    // for each ros node in the current configuration:
    for (auto& ros_node : current_configuration_->ros_nodes) {
      // check if node exists in list of online nodes
      for (auto& online_node : online_nodes_) {
        if (ros_node.name == online_node.first &&
            ros_node.namespace_ == online_node.second) {
          if (std::string(online_node.second) == std::string("/")) {
            online_nodes_str += (std::string("\n  ") + "/" + online_node.first);
          } else {
            online_nodes_str += (std::string("\n  ") + online_node.second +
                                 "/" + online_node.first);
          }
          num_online_nodes_found++;
          break;
        }
      }
    }
  } else {
    // for each ros node in the default configuration:
    num_expected_online_nodes = ros_nodes_.size();
    for (auto& ros_node : ros_nodes_) {
      // check if node exists in list of online nodes
      for (auto& online_node : online_nodes_) {
        if (ros_node.name == online_node.first &&
            ros_node.namespace_ == online_node.second) {
          if (std::string(online_node.second) == std::string("/")) {
            online_nodes_str += (std::string("\n  ") + "/" + online_node.first);
          } else {
            online_nodes_str += (std::string("\n  ") + online_node.second +
                                 "/" + online_node.first);
          }
          num_online_nodes_found++;
          break;
        }
      }
    }
  }

  online_ = (num_online_nodes_found > 0);
  if (num_online_nodes_found != num_online_nodes_prev_) {
    online_nodes_str = std::to_string(num_online_nodes_found) + "/" +
                       std::to_string(num_expected_online_nodes) +
                       std::string(" nodes online") + online_nodes_str;
    RCLCPP_INFO(ros_node_->get_logger(),
                "Status change for node %s, %ld/%ld nodes online",
                name_.c_str(), num_online_nodes_found,
                num_expected_online_nodes);
    num_online_nodes_prev_ = num_online_nodes_found;
    label_->setToolTip(online_nodes_str.c_str());
    if (num_online_nodes_found == 0) {
      label_->setPixmap(red_status_icon_);
      label_->setStyleSheet(QString("color: red"));
    } else if (num_online_nodes_found < num_expected_online_nodes) {
      label_->setPixmap(orange_status_icon_);
      label_->setStyleSheet(QString("color: orange"));
    } else if (num_online_nodes_found == num_expected_online_nodes) {
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
