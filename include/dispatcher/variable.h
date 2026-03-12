#ifndef VARIABLE_H_
#define VARIABLE_H_

#include <map>
#include <vector>

#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QWidget>

#include <yaml-cpp/yaml.h>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <std_msgs/msg/empty.hpp>

namespace dispatcher
{
class DispatcherWidget;
class DispatcherNode;

class Variable : public QWidget
{
  Q_OBJECT;  // must be included to add qt meta information

 public:
  explicit Variable(QWidget* parent, dispatcher::DispatcherNode* ros_node_,
                    const YAML::Node&);
  ~Variable();

  void Enable(bool);
  std::string GetName();
  std::string GetValue();

 public slots:

 private:
  DispatcherWidget* dispatcher_ = nullptr;
  DispatcherNode*   ros_node_   = nullptr;
  QComboBox*        combo_box_  = nullptr;

  std::string                         name_;
  std::vector<std::string>            choices_;
  int                                 index_          = -1;
  QLabel*                             label_          = nullptr;

 signals:
};

}  // namespace dispatcher

#endif
