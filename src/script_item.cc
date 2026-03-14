#include "dispatcher/detail/logic.h"
#include "dispatcher/script_item.h"
#include "dispatcher/dispatcher_widget.h"

#include <stdlib.h>
#include <regex>
#include <string>

#include <QHBoxLayout>
#include <QIcon>
#include <QPixmap>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

/*!
@brief class constructor for ScriptItem
*/
dispatcher::ScriptItem::ScriptItem(QWidget* parent, DispatcherNode* ros_node,
                                   const YAML::Node& node)
    : dispatcher::Item(parent, ros_node, node)
{
  if (node["configurations"]) {
    for (auto& config : node["configurations"]) {
      dispatcher::Item::Config configuration;
      configuration.cmd                = config["cmd"].as<std::string>();
      std::string name                 = config["name"].as<std::string>();
      configuration.configuration_name = name;
      if (config["hostname"]) {
        configuration.hostname = config["hostname"].as<std::string>();
      } else {
        configuration.hostname = "localhost";
      }
      if (config["user"]) {
        configuration.user = config["user"].as<std::string>();
      }

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

  // The GridLayout this ScriptItem is going into will have a space in the 0th
  // row, so just increment the value read from the YAML
  int row    = node["row"].as<int>() + 1;
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
  std::string configured_cmd = current_configuration_->cmd;

  // replace any instances of variables matching pattern $VARIABLE_NAME with its
  // value
  std::vector<std::pair<std::string, std::string>> variable_values;
  for (auto& variable : ros_node_->GetVariables()) {
    variable_values.emplace_back(variable->GetName(), variable->GetValue());
  }
  configured_cmd =
      dispatcher::detail::SubstituteVariables(configured_cmd, variable_values);

  (void)SystemCall(dispatcher::detail::BuildScriptSystemCall(
      configured_cmd, ros_node_->get_cmd_prefix("all"),
      ros_node_->get_environment_variables("all"), use_cmd_prefix_,
      use_terminal_));
}
