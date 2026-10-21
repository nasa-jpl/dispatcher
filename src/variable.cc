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
  allow_while_running_ = node["allow_while_running"].as<bool>(false);
  for (auto& choice : node["choices"]) {
    choices_.push_back(choice.as<std::string>());
  }

  QGridLayout* layout = dispatcher_->get_variable_layout();
  assert(layout);

  label_ = new QLabel(node["label"].as<std::string>(name_).c_str());
  index_ = layout->rowCount();
  layout->addWidget(label_, index_, 0);

  if (node["type"].as<std::string>("choice") == "float") {
    number_input_ = new QDoubleSpinBox();
    number_input_->setObjectName(QString::fromStdString(name_));
    number_input_->setDecimals(node["decimals"].as<int>(3));
    number_input_->setRange(node["min"].as<double>(-1.0),
                           node["max"].as<double>(1.0));
    number_input_->setSingleStep(node["step"].as<double>(0.01));
    number_input_->setValue(node["default"].as<double>(0.0));
    layout->addWidget(number_input_, index_, 1);
  } else {
    combo_box_ = new QComboBox();
    for (auto& choice : choices_) {
      combo_box_->addItem(QString::fromStdString(choice));
    }
    layout->addWidget(combo_box_, index_, 1);
  }
}

/*!
@brief class destructor
*/
dispatcher::Variable::~Variable() {}

void dispatcher::Variable::Enable(bool enable)
{
  if (number_input_) {
    number_input_->setEnabled(enable || allow_while_running_);
  } else {
    combo_box_->setEnabled(enable || allow_while_running_);
  }
}

std::string dispatcher::Variable::GetValue()
{
  return number_input_ ? QString::number(number_input_->value(), 'g', 15).toStdString()
                       : combo_box_->currentText().toStdString();
}

std::string dispatcher::Variable::GetName()
{
  return name_;
}
