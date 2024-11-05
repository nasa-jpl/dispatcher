#include "dispatcher/dispatcher_node.h"
#include "dispatcher/config.h"
#include "dispatcher/dispatcher_widget.h"
#include "dispatcher/ros_process_item.h"
#include "dispatcher/shell_process_item.h"

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
    : casah_node::BaseInterface("dispatcher", "dispatcher"),
      casah_node::EvrInterface("dispatcher", "dispatcher")
{
  widget_ = widget;

  node_graph_ = std::make_shared<rclcpp::node_interfaces::NodeGraph>(
      this->get_node_base_interface().get());

  configurations_["all"] = Configuration();

  InitializeTimerRate();
  DeclareInitParameterString("dispatcher_config_path", "",
                             "Path to dispatcher configuration file");
  this->get_parameter("dispatcher_config_path", dispatcher_config_path_);

  DeclareInitParameterInt("ssh_timeout_sec", 10,
                          "Default timeout for initiating remote ssh sessions");
  this->get_parameter("ssh_timeout_sec", ssh_timeout_sec_);
  ParseConfig();
  CleanupTmuxSessions();
  UpdateConfiguration();
}

void dispatcher::DispatcherNode::ParseConfig()
{
  EVR_DIAGNOSTIC("Parsing dispatcher_config_path: %s",
                 dispatcher_config_path_.c_str());
  YAML::Node root         = YAML::LoadFile(dispatcher_config_path_.c_str());
  auto       dispatcher_w = dynamic_cast<DispatcherWidget*>(widget_);

  // Add other configurations defined in YAML
  auto combo_box = widget_->get_configuration_combo_box();
  if (root["configurations"]) {
    for (const auto& yaml_config : root["configurations"]) {
      std::string name;
      if (yaml_config.IsMap()) {
        name = yaml_config["name"].as<std::string>();
        AddConfiguration(name, yaml_config);
      } else {
        // Also support simpler configuration definition
        name                  = yaml_config.as<std::string>();
        configurations_[name] = dispatcher::DispatcherNode::Configuration();
      }
      combo_box->addItem(QString::fromStdString(name));
    }
  } else {
    combo_box->addItem("default");
  }

  // add default 'all' configuration
  AddConfiguration("all", root);

  workspace_ = root["workspace"].as<std::string>();
  for (const auto& node : root["nodes"]) {
    // Every node defined in YAML is known as a DispatcherItem in DispatcherNode
    // and a Process in DispatcherWidget
    std::string  name = node["name"].as<std::string>();
    QGridLayout* grid_layout;
    if (node["type"]) {
      std::string node_type = node["type"].as<std::string>();
      if (node_type == "category") {
        if (node["items"]) {
          grid_layout = dispatcher_w->add_category_of_processes(name);
          for (const auto& item : node["items"]) {
            node_type = item["type"].as<std::string>();
            AddItem(node_type, item, grid_layout);
          }
        } else {
          EVR_FATAL(
              "Encountered node type %s in YAML but user did not specify an "
              "'items' array that lists all the items to be run. Nothing "
              "additional will be added to dispatcher",
              node_type.c_str());
        }
      } else {
        grid_layout = dispatcher_w->add_single_process(name);
        AddItem(node_type, node, grid_layout);
      }
    } else {
      // For backwards compatibility, assume this is a ROS item
      grid_layout = dispatcher_w->add_single_process(name);
      AddItem("ros", node, grid_layout);
    }
  }

  root["scripts"] ? dispatcher_w->EnableScripts(true)
                  : dispatcher_w->EnableScripts(false);
  for (const auto& script : root["scripts"]) {
    script_items_.push_back(new dispatcher::ScriptItem(widget_, this, script));
  }
  root["variables"] ? dispatcher_w->EnableVariables(true)
                    : dispatcher_w->EnableVariables(false);
  for (const auto& variable : root["variables"]) {
    variables_.push_back(new dispatcher::Variable(widget_, this, variable));
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
    EnableVariables(!online);
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
  for (auto& item : script_items_) {
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
  EVR_ACTIVITY_HI(
      "Removing out-dated tmux session for all dispatcher items...");

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

void dispatcher::DispatcherNode::EnableVariables(bool enable)
{
  for (auto& variable : variables_) {
    variable->Enable(enable);
  }
}

void dispatcher::DispatcherNode::AddItem(std::string       node_type,
                                         const YAML::Node& node,
                                         QGridLayout*      layout)
{
  if (node_type == "shell") {
    dispatcher_items_.push_back(
        new dispatcher::ShellProcessItem(widget_, this, node, layout));
  } else if (node_type == "ros") {
    dispatcher_items_.push_back(
        new dispatcher::RosProcessItem(widget_, this, node, layout));
  } else {
    EVR_WARNING_HI(
        "Encountered node named '%s' with type '%s' in YAML that's "
        "unsupported, it will be "
        "ignored and not added to dispatcher",
        node["name"].as<std::string>().c_str(), node_type.c_str());
  }
}

void dispatcher::DispatcherNode::AddConfiguration(std::string       name,
                                                  const YAML::Node& node)
{
  dispatcher::DispatcherNode::Configuration configuration;
  if (node["environment_variables"]) {
    for (const auto& item : node["environment_variables"]) {
      std::string key                          = item.first.as<std::string>();
      std::string value                        = item.second.as<std::string>();
      configuration.environment_variables[key] = value;
    }
  }
  if (node["cmd_prefix"]) {
    configuration.cmd_prefix = node["cmd_prefix"].as<std::string>();
  }
  configurations_[name] = configuration;
}