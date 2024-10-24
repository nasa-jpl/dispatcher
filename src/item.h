#ifndef ITEM_H_
#define ITEM_H_

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

typedef struct {
  std::string namespace_;
  std::string name;
} RosNodeMonitorConfig;

class Item : public QWidget
{
  Q_OBJECT;  // must be included to add qt meta information

 public:
  explicit Item(QWidget* parent, dispatcher::DispatcherNode* ros_node_,
                const YAML::Node&);
  ~Item();

  void UpdateConfiguration();
  int  SystemCall(std::string cmd, bool verbose = true);

 public slots:
  virtual void StartCb() = 0;

 protected:
  typedef struct {
    std::string                       configuration_name;
    std::vector<RosNodeMonitorConfig> ros_nodes;
    std::string                       cmd;
    std::string                       hostname;
    std::string                       user;
  } Config;

  DispatcherWidget* dispatcher_ = nullptr;
  DispatcherNode*   ros_node_   = nullptr;

  std::string name_;
  bool        use_cmd_prefix_ = true;

  std::map<std::string, Config> configurations_;
  Config*                       current_configuration_ = nullptr;
};

}  // namespace dispatcher

#endif
