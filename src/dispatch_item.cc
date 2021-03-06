#include "dispatcher/dispatch_item.h"

#include <string>

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QString>
#include <QIcon>
#include <QPixmap>
#include <QCheckBox>

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

  QGridLayout* layout = dispatcher_->getLayout();
  index_ = layout->rowCount();
  
  QIcon icon(":/icons/red.png");
  QImage image = QImage(":/icons/red.png").scaledToHeight(20);

  QLabel* label = new QLabel("");
  label->setPixmap(QPixmap::fromImage(image));
  layout->addWidget(label, index_, 1);

  QCheckBox* check_box = new QCheckBox(QString(name_.c_str()), this);
  layout->addWidget(check_box, index_, 0);

  QPushButton* start = new QPushButton("start", this);
  start->setStyleSheet(QString("color: green"));
  layout->addWidget(start, index_, 2);
 
  QPushButton* stop = new QPushButton("stop", this);
  stop->setStyleSheet(QString("color: red"));
  layout->addWidget(stop, index_, 3);
  
  QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  layout->addItem(spacer, index_, 4);
}

/*!
@brief class destructor
*/
dispatcher::DispatchItem::~DispatchItem() {}


