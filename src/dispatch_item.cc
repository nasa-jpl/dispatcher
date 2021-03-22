#include "dispatcher/dispatch_item.h"

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

#include "dispatcher/dispatcher_widget.h"

/*!
@brief class constructor for DispatchItem
*/
dispatcher::DispatchItem::DispatchItem(
    QWidget *parent, dispatcher::DispatcherNode* ros_node, const YAML::Node &node)
    : QWidget(parent) {

  ros_node_ = ros_node;
  name_ = node["name"].as<std::string>();
  node_namespace_ = node["namespace"].as<std::string>();
  node_name_ = node["node_name"].as<std::string>();
  cmd_ = node["cmd"].as<std::string>();

#if 0
  // TODO @kwehage subscribe to Health topics
  // subscribe to health topics
  if (node["topics"]) {
    for (auto &topic : node["topics"]) {
      CFW_INFO("Subscribing to topic: %s", topic.as<std::string>().c_str());
    }
  }
#endif

  dispatcher_ = dynamic_cast<dispatcher::DispatcherWidget*>(parent);
  QGridLayout *layout = dispatcher_->get_layout();
  assert(layout);
  index_ = layout->rowCount();

  QImage image = QImage(":/icons/red.png").scaledToHeight(20);

  label_ = new QLabel("");
  label_->setPixmap(QPixmap::fromImage(image));
  layout->addWidget(label_, index_, 1);

  QCheckBox *check_box = new QCheckBox(QString(name_.c_str()), this);
  layout->addWidget(check_box, index_, 0);

  QPushButton *start = new QPushButton("start", this);
  start->setStyleSheet(QString("color: green"));
  layout->addWidget(start, index_, 2);

  QPushButton *stop = new QPushButton("stop", this);
  stop->setStyleSheet(QString("color: red"));
  layout->addWidget(stop, index_, 3);

  QSpacerItem *spacer =
    new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  layout->addItem(spacer, index_, 4);
}

/*!
@brief class destructor
*/
dispatcher::DispatchItem::~DispatchItem() {}

void dispatcher::DispatchItem::Process() {
  const auto& online_nodes_ = ros_node_->get_online_nodes();

  std::cout << "Processing: " << node_namespace_ << node_name_ << std::endl;

  bool previous_state = online_;
  online_ = false;
  for(auto& online_node : online_nodes_) {
    std::cout << "  => " << online_node.first << online_node.second << std::endl;
    if(node_namespace_ == online_node.first && 
       node_name_ == online_node.second) {

      std::cout << "Online: " << node_namespace_ << node_name_ << std::endl;
      online_ = true;
    }
  }
  if(online_ != previous_state) {
    QImage image;
    if(online_) {
      image = QImage(":/icons/green.png").scaledToHeight(20);
    } else {
      image = QImage(":/icons/red.png").scaledToHeight(20);
    }
    std::cout << "Updating " << node_namespace_ << node_name_ << std::endl;
    label_->setPixmap(QPixmap::fromImage(image));
  }
}

