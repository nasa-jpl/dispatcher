#include "dispatcher/variable.h"

#include "dispatcher/dispatcher_widget.h"

#include <stdlib.h>

#include <string>

#include <QGroupBox>
#include <QHBoxLayout>
#include <QPixmap>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include <yaml-cpp/yaml.h>

/*!
@brief class constructor for Variable
*/
dispatcher::Variable::Variable(QWidget* parent, DispatcherNode* ros_node,
                               const YAML::Node& node)
    : QWidget(parent)
{
  dispatcher_ = dynamic_cast<dispatcher::DispatcherWidget*>(parent);
  ros_node_   = ros_node;
  name_       = node["name"].as<std::string>();
  for (auto& choice : node["choices"]) {
    choices_.push_back(choice.as<std::string>());
  }

  QGridLayout* layout = dispatcher_->get_variable_layout();
  assert(layout);

  label_ = new QLabel(name_.c_str());
  index_ = layout->rowCount();
  layout->addWidget(label_, index_, 0);

  combo_box_ = new QComboBox();

  for (auto& choice : choices_) {
    combo_box_->addItem(QString::fromStdString(choice));
  }
  layout->addWidget(combo_box_, index_, 1);
}

/*!
@brief class destructor
*/
dispatcher::Variable::~Variable() {}

void dispatcher::Variable::Enable(bool enable)
{
  combo_box_->setEnabled(enable);
}

std::string dispatcher::Variable::GetValue()
{
  return combo_box_->currentText().toStdString();
}

std::string dispatcher::Variable::GetName()
{
  return label_->text().toStdString();
}
