#ifndef DISPATCHER_NODE_H_
#define DISPATCHER_NODE_H_

#include "dispatcher/dispatch_item.h"

#include <map>
#include <vector>

#include <rclcpp/node_interfaces/node_graph.hpp>
#include <sensor_msgs/msg/joint_state.hpp>

#include <casah_node/casah_node.hpp>

namespace dispatcher {

class DispatcherWidget;

class DispatcherNode : public CasahNode {

public:
  DispatcherNode(DispatcherWidget*);
  ~DispatcherNode();
  
  double get_target_loop_rate_hz() {
    return target_loop_rate_hz_;
  }

  const std::vector<std::pair<std::string, std::string>>& get_online_nodes() {
    return online_nodes_;
  }
  void JointStatesCb(const std::shared_ptr<sensor_msgs::msg::JointState>) {
    std::cout << "joint states" << std::endl;
  }
  virtual void Process() override;

private:
  std::string dispatcher_config_path_;
  std::vector<dispatcher::DispatchItem*> dispatch_items_;

  std::shared_ptr<rclcpp::node_interfaces::NodeGraph> node_graph_; 
  std::vector<std::pair<std::string, std::string>> online_nodes_;

  void ParseConfig();
  void InitializeSubscribers();

  DispatcherWidget* widget_ = nullptr;
};

}

#endif
