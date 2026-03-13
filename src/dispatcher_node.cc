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

#include <rclcpp/rclcpp.hpp>
#include <rclcpp/node_interfaces/node_base_interface.hpp>

#include <yaml-cpp/yaml.h>

#include <cfw/cfw.h>

/*!
@brief class constructor for DispatcherNode application
*/
dispatcher::DispatcherNode::DispatcherNode(dispatcher::DispatcherWidget* widget)
    : rclcpp::Node("dispatcher_node", rclcpp::NodeOptions().allow_undeclared_parameters(true))
{
  widget_ = widget;

  node_graph_ = std::make_shared<rclcpp::node_interfaces::NodeGraph>(
      this->get_node_base_interface().get());

  this->declare_parameter<double>("target_loop_rate_hz", 100.0);
  this->get_parameter("target_loop_rate_hz", target_loop_rate_hz_);


  this->declare_parameter<std::string>("dispatcher_config_path", "");
  this->get_parameter("dispatcher_config_path", dispatcher_config_path_);

  this->declare_parameter<int>("ssh_timeout_sec", 10);
  this->get_parameter("ssh_timeout_sec", ssh_timeout_sec_);

  this->declare_parameter<std::string>("initial_configuration", "");
  this->get_parameter("initial_configuration", initial_configuration_);

  ParseConfig();
  CleanupTmuxSessions();
  UpdateConfiguration();
}

void dispatcher::DispatcherNode::ParseConfig()
{
  RCLCPP_DEBUG(this->get_logger(), "Parsing dispatcher_config_path: %s",
               dispatcher_config_path_.c_str());
  YAML::Node root         = YAML::LoadFile(dispatcher_config_path_.c_str());
  auto       dispatcher_w = dynamic_cast<DispatcherWidget*>(widget_);

  // Determine how nodes irrelevant to the chosen configuration is displayed
  if (root["hide_unconfigured_processes"]) {
    hide_unconfigured_processes_ =
        root["hide_unconfigured_processes"].as<bool>();
  }

  // Load other configurations defined in YAML and then add them all to
  // combo_box
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
    }
  }
  for (const auto& config : configurations_) {
    if (config.second.icon.empty()) {
      combo_box->addItem(QString::fromStdString(config.first));
    } else {
      combo_box->addItem(QIcon(QString::fromStdString(config.second.icon)),
                         QString::fromStdString(config.first));
    }
  }
  combo_box->setIconSize(QSize(20, 20));

  // Set initial configuration from parameter if specified
  if (!initial_configuration_.empty()) {
    int index = combo_box->findText(QString::fromStdString(initial_configuration_));
    if (index >= 0) {
      combo_box->setCurrentIndex(index);
      RCLCPP_INFO(this->get_logger(), "Set initial_configuration to '%s'",
                  initial_configuration_.c_str());
    } else {
      RCLCPP_WARN(this->get_logger(),
                  "initial_configuration '%s' not found in available configurations",
                  initial_configuration_.c_str());
    }
  } else {
    RCLCPP_DEBUG(this->get_logger(),
                 "No initial_configuration specified, using default");
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
      ItemType node_type = GetItemTypeFromStr(node["type"].as<std::string>());
      if (node_type == CATEGORY) {
        if (node["items"]) {
          grid_layout = dispatcher_w->AddCategoryOfProcesses(name);
          for (const auto& item : node["items"]) {
            node_type = GetItemTypeFromStr(item["type"].as<std::string>());
            AddItem(node_type, item, grid_layout);
          }
        } else {
          RCLCPP_FATAL(
              this->get_logger(),
              "Encountered node type %s in YAML but user did not specify an "
              "'items' array that lists all the items to be run. Nothing "
              "additional will be added to dispatcher",
              ItemTypeToStr(node_type).c_str());
        }
      } else {
        grid_layout = dispatcher_w->AddSingleProcess(name);
        AddItem(node_type, node, grid_layout);
      }
    } else {
      // For backwards compatibility, assume this is a ROS item
      grid_layout = dispatcher_w->AddSingleProcess(name);
      AddItem(ROS, node, grid_layout);
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
  RCLCPP_INFO(this->get_logger(), "Updating configuration to: %s",
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
  // Purposely toggle all collapsible areas to redraw bounding boxes for items
  // that disappear
  for (auto& w : widget_->get_collapsible_widgets()) {
    bool checked = w->get_checked_state();
    w->set_checked_state(!checked);
    w->set_checked_state(checked);
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
  RCLCPP_INFO(this->get_logger(),
              "Stopping all dispatch items and killing tmux sessions...");
  for (auto& item : dispatcher_items_) {
    item->StopCb();
    item->TmuxKillSession();
  }
  RCLCPP_INFO(this->get_logger(),
              "Stopped all dispatch items and killing tmux sessions");
}

void dispatcher::DispatcherNode::CleanupTmuxSessions()
{
  RCLCPP_INFO(this->get_logger(),
              "Removing out-dated tmux session for all dispatcher items...");

  for (auto& item : dispatcher_items_) {
    // Attempt to clean up any local tmux sessions
    if (item->TmuxHasLocalSession()) {
      RCLCPP_WARN(
          this->get_logger(),
          "Prior Tmux session '%s' for item '%s' detected, attempting to clean "
          "up the process safely",
          item->get_tmux_name().c_str(), item->get_name().c_str());
      RCLCPP_WARN(
          this->get_logger(),
          "If you consistently see this warning, contact your SW support "
          "developer");
      item->TmuxSendKeys("C-C");  // SIGINT
      item->TmuxKillSession();
    }
  }
}

void dispatcher::DispatcherNode::SetupTmuxSessions()
{
  RCLCPP_INFO(this->get_logger(),
              "Creating tmux session for all Dispatch Items...");

  for (auto& item : dispatcher_items_) {
    // If the tmux session is already exists, try to refresh session and
    // start it back up from scratch
    if (item->TmuxHasSession()) {
      RCLCPP_WARN(
          this->get_logger(),
          "Prior Tmux session '%s' for item '%s' detected, attempting to clean "
          "up the process safely",
          item->get_tmux_name().c_str(), item->get_name().c_str());
      RCLCPP_WARN(
          this->get_logger(),
          "If you consistently see this warning, contact your SW support "
          "developer");
      item->TmuxSendKeys("C-C");  // SIGINT
      item->TmuxKillSession();
    }

    item->TmuxNewSession();
  }
  RCLCPP_INFO(this->get_logger(),
              "Created Tmux session for all Dispatch Items.");
}

void dispatcher::DispatcherNode::EnableVariables(bool enable)
{
  for (auto& variable : variables_) {
    variable->Enable(enable);
  }
}

void dispatcher::DispatcherNode::AddItem(ItemType          node_type,
                                         const YAML::Node& node,
                                         QGridLayout*      layout)
{
  switch (node_type) {
    case SHELL:
      dispatcher_items_.push_back(
          new dispatcher::ShellProcessItem(widget_, this, node, layout));
      break;
    case ROS:
      dispatcher_items_.push_back(
          new dispatcher::RosProcessItem(widget_, this, node, layout));
      break;
    default:
      RCLCPP_WARN(
          this->get_logger(),
          "Encountered node named '%s' with type '%s' in YAML that's "
          "unsupported, it will be "
          "ignored and not added to dispatcher",
          node["name"].as<std::string>().c_str(),
          ItemTypeToStr(node_type).c_str());
      break;
  }
}

void dispatcher::DispatcherNode::AddConfiguration(const std::string& name,
                                                  const YAML::Node&  node)
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
  if (node["icon"]) {
    configuration.icon = node["icon"].as<std::string>();
  }

  configurations_[name] = configuration;
}

std::string dispatcher::DispatcherNode::ItemTypeToStr(ItemType type)
{
  std::string str_item_type;
  switch (type) {
    case ROS:
      str_item_type = "ros";
      break;
    case SHELL:
      str_item_type = "shell";
      break;
    case CATEGORY:
      str_item_type = "category";
      break;
    default:
      str_item_type = "undef";
      break;
  }
  return str_item_type;
}
dispatcher::DispatcherNode::ItemType
dispatcher::DispatcherNode::GetItemTypeFromStr(std::string type)
{
  ItemType item_type = UNDEF;
  std::transform(type.begin(), type.end(), type.begin(), ::tolower);
  if (type == "shell") {
    item_type = SHELL;
  } else if (type == "ros") {
    item_type = ROS;
  } else if (type == "category") {
    item_type = CATEGORY;
  }
  return item_type;
}
