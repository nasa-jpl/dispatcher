#include "dispatcher/dispatcher_widget.h"
#include "dispatcher/dispatch_item.h"
#include "dispatcher/config.h"

#include <string>
#include <iostream>

#include <QWidget>
#include <QVBoxLayout>
#include <QGroupBox>

#include <yaml-cpp/yaml.h>

#include <cfw/cfw.h>

std::string dispatcher::version() {
  std::string version_major = std::to_string(DISPATCHER_VERSION_MAJOR);
  std::string version_minor = std::to_string(DISPATCHER_VERSION_MINOR);
  std::string version_patch = std::to_string(DISPATCHER_VERSION_PATCH);
  return version_major + std::string(".") + version_minor + std::string(".") +
         version_patch;
}

std::string dispatcher::branch() {
  return std::string(DISPATCHER_GIT_BRANCH);
}

std::string dispatcher::commit() {
  return std::string(DISPATCHER_GIT_COMMIT);
}

/*!
@brief class constructor for DispatcherWidget application
*/
dispatcher::DispatcherWidget::DispatcherWidget(QWidget* parent)
  : QWidget(parent), CasahNode("dispatcher", "dispatcher", 2.0, "/logs", "INFO") {

  this->declare_parameter<std::string>("dispatcher_config_path", "");
  this->get_parameter("dispatcher_config_path", dispatcher_config_path_);

  CFW_INFO("Parsing config file");

  parseConfig(dispatcher_config_path_);

  timer_ = new QTimer;

  QGroupBox* main_box = new QGroupBox;
  main_box->setContentsMargins(QMargins(0, 0, 0, 0));

  QVBoxLayout* layout = new QVBoxLayout(main_box);
  layout->setContentsMargins(QMargins(0, 0, 0, 0));
  layout->setMargin(0);
  layout->setSpacing(0);
}

void dispatcher::DispatcherWidget::parseConfig(const std::string& config) {

  YAML::Node root = YAML::LoadFile(config.c_str());
  const YAML::Node& nodes = root["nodes"];
  for (const auto& node : nodes) {
    dispatch_items_.push_back(new dispatcher::DispatchItem(this, node));
  }
}

/*!
@brief class destructor
*/
dispatcher::DispatcherWidget::~DispatcherWidget() {}


void dispatcher::DispatcherWidget::Process() {}
