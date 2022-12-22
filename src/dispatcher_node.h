#ifndef DISPATCHER_NODE_H_
#define DISPATCHER_NODE_H_

#include "dispatcher/dispatch_item.h"
#include "dispatcher/script_item.h"

#include <map>
#include <vector>

#include <rclcpp/node_interfaces/node_graph.hpp>
#include <sensor_msgs/msg/joint_state.hpp>

#include <casah_node/casah_node.hpp>

namespace dispatcher
{
class DispatcherWidget;

class DispatcherNode : public CasahNode
{
 public:
  DispatcherNode(DispatcherWidget*);
  ~DispatcherNode();

  const std::vector<std::pair<std::string, std::string>>& get_online_nodes()
  {
    return online_nodes_;
  }
  const std::string& get_workspace() { return workspace_; }
  const std::string& get_cmd_prefix() { return cmd_prefix_; }

  virtual void Process() override;
  void         StartChecked();
  void         StopChecked();
  void         StopAll();
  void         UpdateConfiguration();

  // adds public interface to PublishEvr_ message type, used with
  // custom macros above that take a pointer as first argument
  void PublishEvr_(uint8_t type, const char* file, const size_t line,
                   const std::string& message)
  {
    PublishEvr(type, file, line, message);
  }

 private:
  bool                                   last_online_state_ = false;
  std::string                            dispatcher_config_path_;
  std::vector<dispatcher::DispatchItem*> dispatch_items_;
  std::vector<dispatcher::ScriptItem*>   script_items_;
  std::vector<std::string>               configurations_;
  std::string                            cmd_prefix_;
  std::string                            workspace_;
  std::string                            config_;
  std::shared_ptr<rclcpp::node_interfaces::NodeGraph> node_graph_;
  std::vector<std::pair<std::string, std::string>>    online_nodes_;

  void ParseConfig();
  void SetupTmuxSessions();

  DispatcherWidget* widget_ = nullptr;
};

}  // namespace dispatcher

#endif
