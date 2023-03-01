#ifndef SCRIPT_ITEM_H_
#define SCRIPT_ITEM_H_

#include <map>
#include <vector>

#include <QCheckBox>
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

class ScriptItem : public QWidget
{
  Q_OBJECT;  // must be included to add qt meta information

 public:
  explicit ScriptItem(QWidget* parent, dispatcher::DispatcherNode* ros_node_,
                      const YAML::Node&);
  ~ScriptItem();

  bool is_checked();
  void UpdateConfiguration();

 public slots:
  void StartCb();

 private:
  struct ScriptConfig {
    std::string cmd;
  };

  DispatcherWidget* dispatcher_ = nullptr;
  DispatcherNode*   ros_node_   = nullptr;

  std::string                         name_;
  std::string                         icon_;
  int                                 index_          = -1;
  QLabel*                             label_          = nullptr;
  bool                                use_terminal_   = true;
  bool                                use_cmd_prefix_ = true;
  std::map<std::string, ScriptConfig> configurations_;
  ScriptConfig*                       current_configuration_ = nullptr;

 signals:
};

}  // namespace dispatcher

#endif
