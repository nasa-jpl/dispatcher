#include "dispatcher/dispatch_item.h"
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
dispatcher::ScriptItem::ScriptItem(QWidget* parent, const YAML::Node& node)
    : QWidget(parent)
{
  auto dispatcher = dynamic_cast<dispatcher::DispatcherWidget*>(parent);
  name_ = node["name"].as<std::string>();
  cmd_  = node["cmd"].as<std::string>();
  if (node["icon"]) {
    icon_ = node["icon"].as<std::string>();
  } else {
    icon_ = ":/icons/application.png";
  }
  if (node["use_terminal"]) {
    use_terminal_ = node["use_terminal"].as<bool>();
  }
  int row = node["row"].as<int>();
  int column = node["column"].as<int>();

  QGridLayout* layout = dispatcher->get_script_layout();
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
  if(use_terminal_) {
    system_call = "gnome-terminal -- " + cmd_;
  } else {
    system_call = cmd_;
  }

  CFW_INFO("Issuing subprocess call: `%s`", system_call.c_str());
  int result = system(system_call.c_str());
  CFW_INFO("Subprocess call: `%s` returned %d", system_call.c_str(), result);
}
