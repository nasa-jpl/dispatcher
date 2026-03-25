#ifndef ROS_PROCESS_ITEM_H_
#define ROS_PROCESS_ITEM_H_

#include "dispatcher/process_item.h"

namespace dispatcher
{

/*! @brief Process item that monitors one or more ROS graph nodes. */
class RosProcessItem : public ProcessItem
{
  Q_OBJECT  // must be included to add qt meta information

 public:
  /*! @brief Constructs a ROS-backed process item and loads monitor metadata. */
  explicit RosProcessItem(QWidget* parent, DispatcherNode*, const YAML::Node&,
                          QGridLayout*);
  /*! @brief Destroys the ROS process item. */
  ~RosProcessItem();

  /*! @brief Updates the item status from the current ROS graph snapshot. */
  void Process();

 public slots:
  /*! @brief Starts the process after sourcing the workspace setup file. */
  void StartCb();
  /*! @brief Stops the process using the base tmux stop behavior. */
  void StopCb();
  /*! @brief Opens a terminal attached to the process tmux session. */
  void TerminalCb();

 private:
  /*! @brief Default ROS nodes monitored when no config-specific override exists. */
  std::vector<RosNodeMonitorConfig> ros_nodes_;
};

}  // namespace dispatcher

#endif
