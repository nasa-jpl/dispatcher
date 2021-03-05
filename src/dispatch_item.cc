#include "dispatcher/dispatch_item.h"

#include <string>

#include <QWidget>
#include <QVBoxLayout>
#include <QGroupBox>

#include <yaml-cpp/yaml.h>

#include "dispatcher/dispatcher_widget.h"

/*!
@brief class constructor for DispatchItem 
*/
dispatcher::DispatchItem::DispatchItem(QWidget* parent, const YAML::Node& node)
  : QWidget(parent) {
  dispatcher_ = dynamic_cast<dispatcher::DispatcherWidget*>(parent);
  name_ = node["name"].as<std::string>();
  node_namespace_ = node["namespace"].as<std::string>();
  node_name_ = node["node_name"].as<std::string>();
  cmd_ = node["cmd"].as<std::string>();
}

/*!
@brief class destructor
*/
dispatcher::DispatchItem::~DispatchItem() {}


