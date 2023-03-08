#include "dispatcher/dispatcher_item.h"
#include "dispatcher/dispatcher_widget.h"

#include <stdlib.h>

#include <string>

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QPixmap>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include <yaml-cpp/yaml.h>

/*!
@brief class constructor for ScriptItem
*/
dispatcher::ScriptItem::ScriptItem(QWidget* parent, DispatcherNode* ros_node,
                                   const YAML::Node& node)
    : QWidget(parent)
{
  dispatcher_ = dynamic_cast<dispatcher::DispatcherWidget*>(parent);
  ros_node_   = ros_node;
  name_       = node["name"].as<std::string>();

  if (node["cmd"]) {
    // if 'cmd' field exists at root level, create a default "all" configuration
    dispatcher::ScriptItem::ScriptConfig configuration;
    configuration.cmd      = node["cmd"].as<std::string>();
    configurations_["all"] = configuration;
  }
  if (node["configurations"]) {
    for (auto& config : node["configurations"]) {
      dispatcher::ScriptItem::ScriptConfig configuration;
      configuration.cmd     = config["cmd"].as<std::string>();
      std::string name      = config["name"].as<std::string>();
      configurations_[name] = configuration;
    }
  }

  if (node["icon"]) {
    icon_ = node["icon"].as<std::string>();
  } else {
    icon_ = ":/icons/application.png";
  }
  if (node["use_terminal"]) {
    use_terminal_ = node["use_terminal"].as<bool>();
  }
  use_cmd_prefix_ = true;
  if (node["use_cmd_prefix"]) {
    use_cmd_prefix_ = node["use_cmd_prefix"].as<bool>();
  }
  int row    = node["row"].as<int>();
  int column = node["column"].as<int>();

  QGridLayout* layout = dispatcher_->get_script_layout();
  assert(layout);

  std::string str(" ");
  str += name_;
  QPushButton* button = new QPushButton(str.c_str(), this);
  button->setIcon(QIcon(icon_.c_str()));
  button->setIconSize(QSize(20, 20));
  layout->addWidget(button, row, column);
  connect(button, SIGNAL(clicked()), this, SLOT(StartCb()));
}

/*!
@brief class destructor
*/
dispatcher::ScriptItem::~ScriptItem() {}

void dispatcher::ScriptItem::StartCb()
{
  std::string system_call;
  std::string cmd_with_prefix;
  if (use_cmd_prefix_) {
    std::string env_prefix;
    for (auto& it : ros_node_->get_environment_variables("all")) {
      env_prefix += it.first + "=" + it.second + " ";
    }
    assert(current_configuration_);
    cmd_with_prefix = "bash -c \"" + ros_node_->get_cmd_prefix("all") + " " +
                      env_prefix + current_configuration_->cmd + "\"";
  } else {
    cmd_with_prefix = "bash -c \"" + current_configuration_->cmd + "\"";
  }
  if (use_terminal_) {
    system_call = "gnome-terminal -- " + cmd_with_prefix;
  } else {
    system_call = cmd_with_prefix;
  }
  EVR_DIAGNOSTIC_REF(ros_node_, "Issuing subprocess call: `%s`",
                     system_call.c_str());
  int result = system(system_call.c_str());
  EVR_DIAGNOSTIC_REF(ros_node_, "Subprocess call: `%s` returned %d",
                     system_call.c_str(), result);
}

void dispatcher::ScriptItem::UpdateConfiguration()
{
  std::string current_configuration = dispatcher_->get_current_configuration();
  if (configurations_.find(current_configuration) == configurations_.end()) {
    if (configurations_.find("all") == configurations_.end()) {
      // if default 'all' configuration also not found, then disable node
      // SetEnabled(false);
      EVR_ACTIVITY_LO_REF(ros_node_,
                          "Neither 'all' nor '%s' configuration found for "
                          "script '%s'; disabling",
                          current_configuration.c_str(), name_.c_str());

    } else {
      // selected configuration not found, use default configuration
      current_configuration_ = &configurations_["all"];
      EVR_ACTIVITY_LO_REF(ros_node_,
                          "No configurations specified for script '%s', "
                          "using built-in configuration 'all'",
                          name_.c_str());
    }
  } else {
    current_configuration_ = &configurations_[current_configuration];
    EVR_ACTIVITY_LO_REF(ros_node_, "Setting script '%s' to configuration '%s'",
                        name_.c_str(), current_configuration.c_str());
  }
}
