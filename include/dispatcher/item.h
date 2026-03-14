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

/*! @brief ROS node identity pair used when monitoring expected online nodes. */
typedef struct {
  std::string namespace_;
  std::string name;
} RosNodeMonitorConfig;

/*! @brief Base widget type shared by process and script dispatcher items. */
class Item : public QWidget
{
  Q_OBJECT  // must be included to add qt meta information

 public:
  /*! @brief Constructs a dispatcher item from its YAML definition. */
  explicit Item(QWidget* parent, dispatcher::DispatcherNode* ros_node_,
                const YAML::Node&);
  /*! @brief Destroys the dispatcher item. */
  ~Item();

  /*! @brief Selects the effective configuration for the item. */
  void UpdateConfiguration();
  /*!
  @brief Executes a shell command locally or through the configured remote host.
  @param cmd Command to run.
  @param verbose Whether to emit an info log with the expanded command.
  @return Result code returned by `system()`.
  */
  int  SystemCall(std::string cmd, bool verbose = true);

 public slots:
  /*! @brief Starts the item-specific action. */
  virtual void StartCb() = 0;

 protected:
  /*! @brief Per-configuration command and monitoring settings for an item. */
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
