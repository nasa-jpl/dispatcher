#include "dispatcher/item.h"
#include "dispatcher/dispatcher_widget.h"

#include <stdlib.h>
#include <string>

#include <QHBoxLayout>
#include <QIcon>
#include <QPixmap>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

#include <rclcpp/rclcpp.hpp>

/*!
@brief class constructor for Item
*/
dispatcher::Item::Item(QWidget* parent, DispatcherNode* ros_node,
                       const YAML::Node& node)
    : QWidget(parent)
{
  dispatcher_ = dynamic_cast<dispatcher::DispatcherWidget*>(parent);
  ros_node_   = ros_node;
  name_       = node["name"].as<std::string>();

  use_cmd_prefix_ = true;
  if (node["use_cmd_prefix"]) {
    use_cmd_prefix_ = node["use_cmd_prefix"].as<bool>();
  }

  RCLCPP_INFO(ros_node_->get_logger(), "Loading properties for node '%s'",
              name_.c_str());

  if (node["cmd"]) {
    // user has specified a command at root level which applies to all
    // configurations, so create a default 'all' configuration
    dispatcher::Item::Config config;
    config.configuration_name = "all";
    config.cmd                = node["cmd"].as<std::string>();

    if (node["hostname"]) {
      config.hostname = node["hostname"].as<std::string>();
    } else {
      config.hostname = "localhost";
    }

    if (node["user"]) {
      config.user = node["user"].as<std::string>();
    }

    configurations_[config.configuration_name] = config;
  }
}

/*!
@brief class destructor
*/
dispatcher::Item::~Item() {}

void dispatcher::Item::UpdateConfiguration()
{
  std::string current_configuration = dispatcher_->get_current_configuration();

  if (configurations_.find(current_configuration) == configurations_.end()) {
    if (configurations_.find("all") == configurations_.end()) {
      // if default 'all' configuration also not found
      current_configuration_ = nullptr;
      RCLCPP_INFO(ros_node_->get_logger(),
                  "Neither 'all' nor '%s' configuration found for item '%s'; "
                  "disabling",
                  current_configuration.c_str(), name_.c_str());

    } else {
      // selected configuration not found, use default configuration
      current_configuration_ = &configurations_["all"];
      RCLCPP_INFO(ros_node_->get_logger(),
                  "No configurations specified for item '%s', using built-in "
                  "configuration 'all'",
                  name_.c_str());
    }
  } else {
    current_configuration_ = &configurations_[current_configuration];
    RCLCPP_INFO(ros_node_->get_logger(),
                "Setting item '%s' to configuration '%s'", name_.c_str(),
                current_configuration.c_str());
  }
}

int dispatcher::Item::SystemCall(std::string cmd, bool verbose)
{
  RCLCPP_DEBUG(ros_node_->get_logger(), "Issuing subprocess call `%s`",
               cmd.c_str());
  std::string cmd_tmp         = cmd;
  const int   ssh_timeout_sec = ros_node_->get_ssh_timeout_sec();
  if (current_configuration_ &&
      current_configuration_->hostname != "localhost") {
    cmd_tmp =
        "ssh -o PasswordAuthentication=no -o ControlPath=~/.ssh/cm-%r@%h:%p -o "
        "ControlMaster=auto -o ControlPersist=3m -o ConnectTimeout=" +
        std::to_string(ssh_timeout_sec) + " ";
    if (!current_configuration_->user.empty()) {
      cmd_tmp += current_configuration_->user + "@";
    }
    cmd_tmp += current_configuration_->hostname + " \"" + cmd + "\"";
  }
  if (verbose) {
    RCLCPP_INFO(ros_node_->get_logger(), "SystemCall: %s", cmd_tmp.c_str());
  }
  int result = system(cmd_tmp.c_str());
  RCLCPP_DEBUG(ros_node_->get_logger(),
               "Subprocess call `%s` returned result %d", cmd_tmp.c_str(),
               result);
  return result;
}
