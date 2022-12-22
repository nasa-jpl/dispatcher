#ifndef DISPATCH_ITEM_H_
#define DISPATCH_ITEM_H_

#include <unordered_map>
#include <vector>

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include <yaml-cpp/yaml.h>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <std_msgs/msg/empty.hpp>

namespace dispatcher
{
class DispatcherWidget;
class DispatcherNode;

typedef struct {
  std::string namespace_;
  std::string name;
} RosNodeMonitorConfig;

typedef struct {
  std::string                       configuration_name;
  std::vector<RosNodeMonitorConfig> ros_nodes;
  std::string                       cmd;
} DispatchItemConfig;

class DispatchItem : public QWidget
{
  Q_OBJECT  // must be included to add qt meta information

      public : explicit DispatchItem(QWidget* parent, DispatcherNode*,
                                     const YAML::Node&);
  ~DispatchItem();

  // get/set methods
  bool        is_online() { return online_; }
  std::string get_tmux_name() { return tmux_name_; };
  std::string get_name() { return name_; };
  bool        is_checked();

  void SetEnabled(bool);
  void Process();
  void UpdateConfiguration();
  bool TmuxKillSession();
  bool TmuxNewSession();
  void TmuxSendKeys(std::string cmd_str);
  int  SystemCall(std::string cmd);
  bool TmuxHasSession();

 public slots:
  void StartCb();
  void StopCb();
  void TerminalCb();

 private:
  DispatcherWidget* dispatcher_ = nullptr;
  DispatcherNode*   ros_node_   = nullptr;
  std::string       name_;

  // default ros nodes to monitor / applies to all configurations
  // if not overridden with configuration specific nodes to monitor
  std::vector<RosNodeMonitorConfig> ros_nodes_;

  std::unordered_map<std::string, DispatchItemConfig> configurations_;
  DispatchItemConfig* current_configuration_ = nullptr;

  bool         enabled_ = true;
  std::string  tmux_name_;
  std::string  stop_tmux_cmd_;
  int          index_  = -1;
  bool         online_ = false;
  size_t       num_online_nodes_prev_;
  bool         use_cmd_prefix_ = true;
  QLabel*      label_          = nullptr;
  QPixmap      red_status_icon_;
  QPixmap      orange_status_icon_;
  QPixmap      green_status_icon_;
  QPixmap      grey_status_icon_;
  QCheckBox*   check_box_ = nullptr;
  QPushButton* start_     = nullptr;
  QPushButton* stop_      = nullptr;
  QPushButton* terminal_  = nullptr;

 signals:
};

}  // namespace dispatcher

#endif
