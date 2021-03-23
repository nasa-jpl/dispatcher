#ifndef DISPATCHER_WIDGET_H_
#define DISPATCHER_WIDGET_H_

#include "dispatcher/dispatch_item.h"
#include "dispatcher/dispatcher_node.h"

#include <map>
#include <vector>

#include <QGroupBox>
#include <QGridLayout>
#include <QWidget>
#include <QTimer>
#include <QSocketNotifier>
#include <QCoreApplication>

#include <casah_node/casah_node.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp/node_interfaces/node_graph.hpp>

#include <sensor_msgs/msg/joint_state.hpp>

namespace dispatcher {

  std::string version();
  std::string branch();
  std::string commit();  
  
class DispatcherWidget : public QWidget {

  Q_OBJECT // must be included to add qt meta information

public:
  explicit DispatcherWidget(QWidget* parent = 0);
  ~DispatcherWidget();
  QGridLayout* get_layout() {
    return layout_;
  }
  const std::vector<std::pair<std::string, std::string>>& get_online_nodes() {
    return online_nodes_;
  }
  void Quit() {
    QCoreApplication::quit();
  }

public slots:
  void Process();
  void StartAllCheckedCb();
  void StopAllCheckedCb();

private:

  std::shared_ptr<dispatcher::DispatcherNode> ros_node_;
  rclcpp::executors::SingleThreadedExecutor ros_executor_;

  QTimer* timer_ = nullptr;
  QGridLayout* layout_ = nullptr;
  std::string dispatcher_config_path_;
  std::vector<dispatcher::DispatchItem*> dispatch_items_;

  std::shared_ptr<rclcpp::node_interfaces::NodeGraph> node_graph_; 
  std::vector<std::pair<std::string, std::string>> online_nodes_;

  QSize minimumSizeHint() const {
    return QSize(30, 30);
  }
  QSize sizeHint() const {
    return QSize(30, 30);
  }
  void InitializeLayout();

signals:
};

}

#endif
