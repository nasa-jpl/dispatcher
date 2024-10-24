#ifndef ROS_PROCESS_ITEM_H_
#define ROS_PROCESS_ITEM_H_

#include "dispatcher/process_item.h"

namespace dispatcher
{

class RosProcessItem : public ProcessItem
{
  Q_OBJECT;  // must be included to add qt meta information

 public:
  explicit RosProcessItem(QWidget* parent, DispatcherNode*, const YAML::Node&);
  ~RosProcessItem();

  // Uses rqt to poll for aliveness of ROS nodes
  // Also prints a more verbose EVR to report status of ros_nodes
  void Process();

 public slots:
  // Additionally does a source install/setup.bash before running commands
  void StartCb();
  void StopCb();
  void TerminalCb();

 private:
  // default ros nodes to monitor / applies to all configurations
  // if not overridden with configuration specific nodes to monitor
  std::vector<RosNodeMonitorConfig> ros_nodes_;
};

}  // namespace dispatcher

#endif
