#ifndef DISPATCH_ITEM_H_
#define DISPATCH_ITEM_H_

#include <vector>

#include <QGroupBox>
#include <QLabel>
#include <QWidget>
#include <QCheckBox>

#include <yaml-cpp/yaml.h>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/empty.hpp>
#include <sensor_msgs/msg/joint_state.hpp>

namespace dispatcher {

class DispatcherWidget;
class DispatcherNode;
class HealthMonitorItem;

class DispatchItem : public QWidget {

  Q_OBJECT // must be included to add qt meta information

public:
  explicit DispatchItem(QWidget* parent, DispatcherNode*, const YAML::Node&);
  ~DispatchItem();
  
  void Process();
  bool is_checked();

public slots:
  void StartCb();
  void StopCb();
  void TerminalCb();

private:

  DispatcherWidget* dispatcher_ = nullptr;
  DispatcherNode* ros_node_ = nullptr;
  std::string name_;
  std::string node_name_;
  std::string node_namespace_;
  std::string cmd_;
  int index_ = -1;
  bool online_ = false;
  QLabel* label_ = nullptr;
  QPixmap red_status_icon_;
  QPixmap green_status_icon_;
  QCheckBox* check_box_ = nullptr;
signals:
};

}

#endif
