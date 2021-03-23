#include "dispatcher/dispatcher_node.h"
#include "dispatcher/dispatcher_widget.h"
#include "dispatcher/dispatch_item.h"
#include "dispatcher/config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <iostream>

#include <QWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QSpacerItem>
#include <QSocketNotifier>
#include <QCoreApplication>

#include <rclcpp/node_interfaces/node_base_interface.hpp>

#include <yaml-cpp/yaml.h>

#include <cfw/cfw.h>

/*!
@brief class constructor for DispatcherNode application
*/
dispatcher::DispatcherNode::DispatcherNode(dispatcher::DispatcherWidget* widget)
  : CasahNode("dispatcher", "dispatcher", 2.0, "/logs", "INFO") {

  widget_ = widget;

  // declare ros parameters
  this->declare_parameter<std::string>("dispatcher_config_path", "");
  this->get_parameter("dispatcher_config_path", dispatcher_config_path_);

  node_graph_ = 
    std::make_shared<rclcpp::node_interfaces::NodeGraph>(
      this->get_node_base_interface().get());

  ParseConfig();
  InitializeTimer();
}


void dispatcher::DispatcherNode::ParseConfig() {

  CFW_INFO("Parsing dispatcher_config_path: %s", dispatcher_config_path_.c_str());
  YAML::Node root = YAML::LoadFile(dispatcher_config_path_.c_str());
  
  workspace_ = root["workspace"].as<std::string>();
  const YAML::Node& nodes = root["nodes"];
   
  for (const auto& node : nodes) {
    dispatch_items_.push_back(new dispatcher::DispatchItem(widget_, this, node));
  }
}


/*!
@brief class destructor
*/
dispatcher::DispatcherNode::~DispatcherNode() {}

void dispatcher::DispatcherNode::Process() {
  if(rclcpp::ok()) {
    online_nodes_ = node_graph_->get_node_names_and_namespaces();
    for(auto& item : dispatch_items_) {
      item->Process();
    }
  } else {
    CFW_INFO("SHUTTING DOWN");
    widget_->Quit();
  }
}


void dispatcher::DispatcherNode::StartChecked() {
  for(auto& item : dispatch_items_) {
    if(item->is_checked()) {
      item->StartCb();
    }
  }
}


void dispatcher::DispatcherNode::StopChecked() {
  for(auto& item : dispatch_items_) {
    if(item->is_checked()) {
      item->StopCb();
    }
  }
}

void dispatcher::DispatcherNode::StopAll() {
  for(auto& item : dispatch_items_) {
    item->StopCb();
  }
}

