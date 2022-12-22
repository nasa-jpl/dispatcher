#include "dispatcher/config.h"
#include "dispatcher/dispatch_item.h"
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
  ParseConfig();
  SetupTmuxSessions();
}

void dispatcher::DispatcherNode::ParseConfig()
{
  EVR_DIAGNOSTIC("Parsing dispatcher_config_path: %s",
                 dispatcher_config_path_.c_str());
  YAML::Node root = YAML::LoadFile(dispatcher_config_path_.c_str());

  auto combo_box = widget_->get_configuration_combo_box();
  if (root["configurations"]) {
    for (const auto& configuration : root["configurations"]) {
      combo_box->addItem(
          QString::fromStdString(configuration.as<std::string>()));
    }
  } else {
    combo_box->addItem("default");
  }

  if (root["cmd_prefix"]) {
    cmd_prefix_ = root["cmd_prefix"].as<std::string>();
  }

  workspace_ = root["workspace"].as<std::string>();
  if (root["config"]) {
    config_ = root["config"].as<std::string>();
  }
  for (const auto& node : root["nodes"]) {
    dispatch_items_.push_back(
       new dispatcher::DispatchItem(widget_, this, node));
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
  if (!config_.empty()) {
    title += " - " + QString(config_.c_str());
  }
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
  for (auto& item : dispatch_items_) {
    if (item->is_online()) {
      online = true;
      break;
    }
  }
  if (online != last_online_state_) {
    widget_->get_configuration_combo_box()->setEnabled(!online);
  }
  last_online_state_ = online;

  for (auto& item : dispatch_items_) {
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
  for (auto& item : dispatch_items_) {
    item->UpdateConfiguration();
  }
}

void dispatcher::DispatcherNode::StartChecked()
{
  for (auto& item : dispatch_items_) {
    if (item->is_checked()) {
      item->StartCb();
    }
  }
}

void dispatcher::DispatcherNode::StopChecked()
{
  for (auto& item : dispatch_items_) {
    if (item->is_checked()) {
      item->StopCb();
    }
  }
}

void dispatcher::DispatcherNode::StopAll()
{
  EVR_ACTIVITY_HI("Stopping all dispatch items and killing tmux sessions...");
  for (auto& item : dispatch_items_) {
    item->StopCb();
    item->TmuxKillSession();
  }
  EVR_ACTIVITY_LO("Stoped all dispatch items and killing tmux sessions");
}

void dispatcher::DispatcherNode::SetupTmuxSessions()
{
  EVR_ACTIVITY_HI("Creating tmux session for all Dispatch Items...");

  for (auto& item : dispatch_items_) {
    // If the tmux session is already exists, try to clean it up and
    // start it back up from scratch
    if (item->TmuxHasSession()) {
      EVR_WARNING_HI(
          "Prior Tmux session '%s' for item '%s' detected, attempting to clean "
          "up the "
          "process safely",
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
