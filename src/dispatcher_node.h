#ifndef DISPATCHER_NODE_H_
#define DISPATCHER_NODE_H_

#include "dispatcher/process_item.h"
#include "dispatcher/script_item.h"
#include "dispatcher/variable.h"

#include <map>
#include <unordered_map>
#include <vector>

#include <QGridLayout>
#include <rclcpp/node_interfaces/node_graph.hpp>
#include <sensor_msgs/msg/joint_state.hpp>

#include "casah_node/evr_interface.hpp"

namespace dispatcher
{
class DispatcherWidget;

class DispatcherNode : public casah_node::EvrInterface
{
 public:
  DispatcherNode(DispatcherWidget*);
  ~DispatcherNode();

  struct Configuration {
    std::map<std::string, std::string> environment_variables;
    std::string                        cmd_prefix;
    std::string                        icon = "";
  };

  const std::vector<std::pair<std::string, std::string>>& get_online_nodes()
  {
    return online_nodes_;
  }
  const std::string& get_workspace() { return workspace_; }
  const std::string& get_cmd_prefix(const std::string& configuration)
  {
    // check if configuration exists
    if (configurations_.find(configuration) == configurations_.end()) {
      // configuration was not found, return default 'all' configuration
      return configurations_["all"].cmd_prefix;
    } else {
      // check if specified configuration is empty
      if (configurations_[configuration].cmd_prefix.empty()) {
        // if empty, override with default 'all' configuration
        return configurations_["all"].cmd_prefix;
      } else {
        return configurations_[configuration].cmd_prefix;
      }
    }
  }
  const std::map<std::string, std::string>& get_environment_variables(
      const std::string& configuration)
  {
    // check if configuration exists
    if (configurations_.find(configuration) == configurations_.end()) {
      // configuration was not found, return default 'all' configuration
      return configurations_["all"].environment_variables;
    } else {
      // check if specified configuration is empty
      if (configurations_[configuration].environment_variables.empty()) {
        // if empty, override with default 'all' configuration
        return configurations_["all"].environment_variables;
      } else {
        return configurations_[configuration].environment_variables;
      }
    }
  }
  int          get_ssh_timeout_sec() { return ssh_timeout_sec_; }
  virtual void Process() override;
  void         StartChecked();
  void         StopChecked();
  void         StopAll();
  void         UpdateConfiguration();
  void         EnableVariables(bool);
  const std::vector<dispatcher::Variable*>& GetVariables()
  {
    return variables_;
  }
  double GetTimerRate() { return casah_node::BaseInterface::GetTimerRate(); }

 private:
  bool                                  last_online_state_        = false;
  bool                                  tmux_sessions_configured_ = false;
  std::string                           dispatcher_config_path_;
  int                                   ssh_timeout_sec_ = 10;
  std::vector<dispatcher::ProcessItem*> dispatcher_items_;
  std::vector<dispatcher::ScriptItem*>  script_items_;
  std::vector<dispatcher::Variable*>    variables_;
  std::string                           workspace_;
  std::shared_ptr<rclcpp::node_interfaces::NodeGraph> node_graph_;
  std::vector<std::pair<std::string, std::string>>    online_nodes_;
  std::map<std::string, Configuration>                configurations_;

  void              ParseConfig();
  void              AddConfiguration(std::string, const YAML::Node&);
  void              AddItem(std::string, const YAML::Node&, QGridLayout*);
  void              SetupTmuxSessions();
  void              CleanupTmuxSessions();
  DispatcherWidget* widget_ = nullptr;
};

}  // namespace dispatcher

#endif
