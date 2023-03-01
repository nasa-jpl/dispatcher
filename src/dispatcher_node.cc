#include "dispatcher/config.h"
#include "dispatcher/dispatcher_item.h"
#include "dispatcher/dispatcher_node.h"
#include "dispatcher/dispatcher_widget.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include <QCoreApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QSocketNotifier>
#include <QSpacerItem>
#include <QWidget>

#include <rclcpp/node_interfaces/node_base_interface.hpp>

#include <yaml-cpp/yaml.h>

#include <cfw/cfw.h>

/*!
@brief class constructor for DispatcherNode application
*/
dispatcher::DispatcherNode::DispatcherNode(dispatcher::DispatcherWidget* widget)
    : CasahNode("dispatcher", "dispatcher")
{
  widget_ = widget;

  node_graph_ = std::make_shared<rclcpp::node_interfaces::NodeGraph>(
      this->get_node_base_interface().get());

  InitializeTimerRate();
  DeclareInitParameterString("dispatcher_config_path", "",
                             "Path to dispatcher configuration file");
  this->get_parameter("dispatcher_config_path", dispatcher_config_path_);
  
  DeclareInitParameterInt(
    "ssh_timeout_sec", 10, "Default timeout for initiating remote ssh sessions"
  );
  this->get_parameter("ssh_timeout_sec", ssh_timeout_sec_);
  ParseConfig();
  CleanupTmuxSessions();
}

void dispatcher::DispatcherNode::ParseConfig()
{
  EVR_DIAGNOSTIC("Parsing dispatcher_config_path: %s",
                 dispatcher_config_path_.c_str());
  YAML::Node root = YAML::LoadFile(dispatcher_config_path_.c_str());

  auto combo_box = widget_->get_configuration_combo_box();
  if (root["configurations"]) {
    for (const auto& yaml_config : root["configurations"]) {
      std::string name;
      if(yaml_config["name"]) {
        name = yaml_config["name"].as<std::string>();
        dispatcher::DispatcherNode::Configuration configuration;
        if(yaml_config["environment_variables"]) {
          for(const auto& item : root["environment_variables"]) {
            std::string key = item.first.as<std::string>();
            std::string value = item.second.as<std::string>();
            configuration.environment_variables[key] = value;
          }
        }
        if(yaml_config["cmd_prefix"]) {
          configuration.cmd_prefix = yaml_config["cmd_prefix"].as<std::string>();
        }
        configurations_[name] = configuration;
      } else {
        name = yaml_config.as<std::string>();
        configurations_[name] = dispatcher::DispatcherNode::Configuration();
      }
      combo_box->addItem(QString::fromStdString(name));
    }
  } else {
    combo_box->addItem("default");
  }

  // add default 'all' configuration
  dispatcher::DispatcherNode::Configuration configuration;
  if (root["cmd_prefix"]) {
    configuration.cmd_prefix = root["cmd_prefix"].as<std::string>();
  }
  if (root["environment_variables"]) {
    for(const auto& item : root["environment_variables"]) {
      std::string key = item.first.as<std::string>();
      std::string value = item.second.as<std::string>();
      configuration.environment_variables[key] = value;
    }
  }
  configurations_["all"] = configuration;

  workspace_ = root["workspace"].as<std::string>();
  for (const auto& node : root["nodes"]) {
    dispatcher_items_.push_back(
       new dispatcher::DispatcherItem(widget_, this, node));
  }

  auto dispatcher = dynamic_cast<DispatcherWidget*>(widget_);
  if (root["scripts"]) {
    dispatcher->EnableScripts(true);
  } else {
    dispatcher->EnableScripts(false);
  }
  for (const auto& script : root["scripts"]) {
    script_items_.push_back(new dispatcher::ScriptItem(widget_, this, script));
  }

  QString title = "dispatcher - " + QString(workspace_.c_str());
  widget_->setWindowTitle(title);
}

/*!
@brief class destructor
*/
dispatcher::DispatcherNode::~DispatcherNode() {}

void dispatcher::DispatcherNode::Process()
{
  online_nodes_ = node_graph_->get_node_names_and_namespaces();

  bool online = false;
  for (auto& item : dispatcher_items_) {
    if (item->is_online()) {
      online = true;
      break;
    }
  }
  if (online != last_online_state_) {
    widget_->get_configuration_combo_box()->setEnabled(!online);
  }
  last_online_state_ = online;

  for (auto& item : dispatcher_items_) {
    item->Process();
  }
}

void dispatcher::DispatcherNode::UpdateConfiguration()
{
  EVR_ACTIVITY_HI("Updating configuration to: %s",
                  widget_->get_configuration_combo_box()
                      ->currentText()
                      .toStdString()
                      .c_str());

  online_nodes_ = node_graph_->get_node_names_and_namespaces();
  for (auto& item : dispatcher_items_) {
    item->UpdateConfiguration();
  }
  for (auto&item : script_items_) {
    item->UpdateConfiguration();
  }
}

void dispatcher::DispatcherNode::StartChecked()
{
  for (auto& item : dispatcher_items_) {
    if (item->is_checked()) {
      item->StartCb();
    }
  }
}

void dispatcher::DispatcherNode::StopChecked()
{
  for (auto& item : dispatcher_items_) {
    if (item->is_checked()) {
      item->StopCb();
    }
  }
}

void dispatcher::DispatcherNode::StopAll()
{
  EVR_ACTIVITY_HI("Stopping all dispatch items and killing tmux sessions...");
  for (auto& item : dispatcher_items_) {
    item->StopCb();
    item->TmuxKillSession();
  }
  EVR_ACTIVITY_LO("Stopped all dispatch items and killing tmux sessions");
}

void dispatcher::DispatcherNode::CleanupTmuxSessions()
{
  EVR_ACTIVITY_HI("Removing out-dated tmux session for all dispatcher items...");

  for (auto& item : dispatcher_items_) {
    // Attempt to clean up any local tmux sessions
    if (item->TmuxHasLocalSession()) {
      EVR_WARNING_HI(
          "Prior Tmux session '%s' for item '%s' detected, attempting to clean "
          "up the process safely",
          item->get_tmux_name().c_str(), item->get_name().c_str());
      EVR_WARNING_LO(
          "If you consistently see this warning, contact your SW support "
          "developer");
      item->TmuxSendKeys("C-C");  // SIGINT
      item->TmuxKillSession();
    }
  }
}

void dispatcher::DispatcherNode::SetupTmuxSessions()
{
  EVR_ACTIVITY_HI("Creating tmux session for all Dispatch Items...");

  for (auto& item : dispatcher_items_) {
    // If the tmux session is already exists, try to refresh session and
    // start it back up from scratch
    if (item->TmuxHasSession()) {
      EVR_WARNING_HI(
          "Prior Tmux session '%s' for item '%s' detected, attempting to clean "
          "up the process safely",
          item->get_tmux_name().c_str(), item->get_name().c_str());
      EVR_WARNING_LO(
          "If you consistently see this warning, contact your SW support "
          "developer");
      item->TmuxSendKeys("C-C");  // SIGINT
      item->TmuxKillSession();
    }

    item->TmuxNewSession();
  }
  EVR_ACTIVITY_LO("Created Tmux session for all Dispatch Items.");
}
