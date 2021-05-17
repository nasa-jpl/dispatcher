#include "dispatcher/dispatcher_node.h"
#include "dispatcher/config.h"
#include "dispatcher/dispatch_item.h"
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
    : CasahNode("dispatcher", "dispatcher", 2.0, "/logs", "INFO")
{
  widget_ = widget;

  // declare ros parameters
  this->declare_parameter<std::string>("dispatcher_config_path", "");
  this->get_parameter("dispatcher_config_path", dispatcher_config_path_);

  node_graph_ = std::make_shared<rclcpp::node_interfaces::NodeGraph>(
      this->get_node_base_interface().get());

  ParseConfig();
  SetupTmuxSessions();
  InitializeTimer();
}

void dispatcher::DispatcherNode::ParseConfig()
{
  EVR_DIAGNOSTIC("Parsing dispatcher_config_path: %s",
                 dispatcher_config_path_.c_str());
  YAML::Node root = YAML::LoadFile(dispatcher_config_path_.c_str());

  workspace_              = root["workspace"].as<std::string>();
  int dispatch_item_index = 1;
  for (const auto& node : root["nodes"]) {
    dispatch_items_.push_back(new dispatcher::DispatchItem(
        widget_, this, node, dispatch_item_index++));
  }

  auto dispatcher = dynamic_cast<DispatcherWidget*>(widget_);
  if (root["scripts"]) {
    dispatcher->EnableScripts(true);
  } else {
    dispatcher->EnableScripts(false);
  }
  for (const auto& script : root["scripts"]) {
    script_items_.push_back(new dispatcher::ScriptItem(widget_, script));
  }

  widget_->setWindowTitle("dispatcher - " + QString(workspace_.c_str()));
}

/*!
@brief class destructor
*/
dispatcher::DispatcherNode::~DispatcherNode() {}

void dispatcher::DispatcherNode::Process()
{
  online_nodes_ = node_graph_->get_node_names_and_namespaces();
  for (auto& item : dispatch_items_) {
    item->Process();
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
  EVR_ACTIVITY_HI( 
    "Stopping all dispatch items and killing tmux sessions...");
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
          item->GetTmuxName().c_str(), item->GetName().c_str());
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
