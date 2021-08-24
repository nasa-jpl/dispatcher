#ifndef DISPATCHER_NODE_H_
#define DISPATCHER_NODE_H_

#include "dispatcher/dispatch_item.h"
#include "dispatcher/script_item.h"

#include <map>
#include <vector>

#include <rclcpp/node_interfaces/node_graph.hpp>
#include <sensor_msgs/msg/joint_state.hpp>

#include <casah_node/casah_node.hpp>

// Create variants of EVR_PRINT macros that allow calling ROS2 PublishEVR
// method from supporting code. This one-off solution allows 
// Dispatcher to log EVR activity from supporting code that is not derived 
// from the casah_node base class. 
#define EVR_FATAL_PTR(ptr, fmt, ...) \
do { \
  int length = std::snprintf(nullptr, 0, fmt, ##__VA_ARGS__); \
  assert(length >= 0); \
  char buf[length + 1]; \
  std::snprintf(buf, length + 1, fmt, ##__VA_ARGS__); \
  std::string msg(buf); \
  CFW_ERROR(fmt, ##__VA_ARGS__); \
  ptr->PublishEvr_(casah_msgs::msg::Evr::FATAL, __FILE__, __LINE__, msg); \
} while(0)

#define EVR_DIAGNOSTIC_PTR(ptr, fmt, ...) \
do { \
  int length = std::snprintf(nullptr, 0, fmt, ##__VA_ARGS__); \
  assert(length >= 0); \
  char buf[length + 1]; \
  std::snprintf(buf, length + 1, fmt, ##__VA_ARGS__); \
  std::string msg(buf); \
  CFW_DEBUG(fmt, ##__VA_ARGS__); \
  ptr->PublishEvr_(casah_msgs::msg::Evr::DIAGNOSTIC, __FILE__, __LINE__, msg); \
} while(0)

#define EVR_COMMAND_PTR(ptr, fmt, ...) \
do { \
  int length = std::snprintf(nullptr, 0, fmt, ##__VA_ARGS__); \
  assert(length >= 0); \
  char buf[length + 1]; \
  std::snprintf(buf, length + 1, fmt, ##__VA_ARGS__); \
  std::string msg(buf); \
  CFW_INFO(fmt, ##__VA_ARGS__); \
  ptr->PublishEvr_(casah_msgs::msg::Evr::COMMAND, __FILE__, __LINE__, msg); \
} while(0)

#define EVR_WARNING_HI_PTR(ptr, fmt, ...) \
do { \
  CFW_WARN(fmt, ##__VA_ARGS__); \
  int length = std::snprintf(nullptr, 0, fmt, ##__VA_ARGS__); \
  assert(length >= 0); \
  char buf[length + 1]; \
  std::snprintf(buf, length + 1, fmt, ##__VA_ARGS__); \
  std::string msg(buf); \
  ptr->PublishEvr_(casah_msgs::msg::Evr::WARNING_HI, __FILE__, __LINE__, msg); \
} while(0)

#define EVR_WARNING_LO_PTR(ptr, fmt, ...) \
do { \
  CFW_WARN(fmt, ##__VA_ARGS__); \
  int length = std::snprintf(nullptr, 0, fmt, ##__VA_ARGS__); \
  assert(length >= 0); \
  char buf[length + 1]; \
  std::snprintf(buf, length + 1, fmt, ##__VA_ARGS__); \
  std::string msg(buf); \
  ptr->PublishEvr_(casah_msgs::msg::Evr::WARNING_LO, __FILE__, __LINE__, msg); \
} while(0)

#define EVR_ACTIVITY_HI_PTR(ptr, fmt, ...) \
do { \
  CFW_INFO(fmt, ##__VA_ARGS__); \
  int length = std::snprintf(nullptr, 0, fmt, ##__VA_ARGS__); \
  assert(length >= 0); \
  char buf[length + 1]; \
  std::snprintf(buf, length + 1, fmt, ##__VA_ARGS__); \
  std::string msg(buf); \
  ptr->PublishEvr_(casah_msgs::msg::Evr::ACTIVITY_HI, __FILE__, __LINE__, msg); \
} while(0)

#define EVR_ACTIVITY_LO_PTR(ptr, fmt, ...) \
do { \
  CFW_INFO(fmt, ##__VA_ARGS__); \
  int length = std::snprintf(nullptr, 0, fmt, ##__VA_ARGS__); \
  assert(length >= 0); \
  char buf[length + 1]; \
  std::snprintf(buf, length + 1, fmt, ##__VA_ARGS__); \
  std::string msg(buf); \
  ptr->PublishEvr_(casah_msgs::msg::Evr::ACTIVITY_LO, __FILE__, __LINE__, msg); \
} while(0)


namespace dispatcher
{
class DispatcherWidget;

class DispatcherNode : public CasahNode
{
 public:
  DispatcherNode(DispatcherWidget*);
  ~DispatcherNode();

  double get_target_loop_rate_hz() { return target_loop_rate_hz_; }
  const std::vector<std::pair<std::string, std::string>>& get_online_nodes()
  {
    return online_nodes_;
  }
  const std::string& get_workspace() { return workspace_; }

  virtual void Process() override;
  void         StartChecked();
  void         StopChecked();
  void         StopAll();

  // adds public interface to PublishEvr_ message type, used with
  // custom macros above that take a pointer as first argument
  void PublishEvr_(uint8_t type, const char* file, const size_t line, 
                   const std::string& message) {
    PublishEvr(type, file, line, message);
  }
 private:
  std::string                            dispatcher_config_path_;
  std::vector<dispatcher::DispatchItem*> dispatch_items_;
  std::vector<dispatcher::ScriptItem*>   script_items_;
  std::string                            workspace_;
  std::string                            config_;
  std::shared_ptr<rclcpp::node_interfaces::NodeGraph> node_graph_;
  std::vector<std::pair<std::string, std::string>>    online_nodes_;

  void ParseConfig();
  void InitializeSubscribers();
  void SetupTmuxSessions();

  DispatcherWidget* widget_ = nullptr;
};

}  // namespace dispatcher

#endif
