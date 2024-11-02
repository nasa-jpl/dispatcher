#ifndef DISPATCHER_WIDGET_H_
#define DISPATCHER_WIDGET_H_

#include "dispatcher/dispatcher_node.h"

#include <exception>
#include <map>
#include <vector>

#include <QComboBox>
#include <QCoreApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QSocketNotifier>
#include <QSplitter>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

#include <rclcpp/node_interfaces/node_graph.hpp>
#include <rclcpp/rclcpp.hpp>
#include "casah_node/evr_interface.hpp"

#include <sensor_msgs/msg/joint_state.hpp>

namespace dispatcher
{
std::string version();
std::string branch();
std::string commit();

class DispatcherException : public std::exception
{
 private:
  const char* message_;

 public:
  explicit DispatcherException(const char* message) { message_ = message; };
  const char* what() const noexcept override { return message_; }
};

class DispatcherCategoryWidget : public QGroupBox
{
  Q_OBJECT  // must be included to add qt meta information

      public : DispatcherCategoryWidget(QWidget*    parent        = 0,
                                        std::string category_name = "Default");
  ~DispatcherCategoryWidget();

  QGridLayout* grid_layout_;
  QGridLayout* get_grid_layout() { return grid_layout_; }

 public slots:
  void ToggleCb(bool);

 private:
  QToolButton*        toggle_button_   = nullptr;
  QGroupBox*          toggle_groupbox_ = nullptr;
  QPropertyAnimation* animation_       = nullptr;
};

class DispatcherWidget : public QScrollArea
{
  Q_OBJECT  // must be included to add qt meta information

      public : explicit DispatcherWidget(QWidget*    parent = 0,
                                         std::string dispatcher_lock_file_path =
                                             "/tmp/dispatcher.lock");
  ~DispatcherWidget();

  // get methods
  QGridLayout* get_script_layout() { return script_layout_; }
  QGridLayout* get_variable_layout() { return variable_layout_; }
  std::shared_ptr<dispatcher::DispatcherNode> get_ros_node()
  {
    return ros_node_;
  }
  const std::vector<std::pair<std::string, std::string>>& get_online_nodes()
  {
    return online_nodes_;
  }
  QComboBox*  get_configuration_combo_box() { return configuration_combo_box_; }
  std::string get_current_configuration()
  {
    return configuration_combo_box_->currentText().toStdString();
  }

  // Allows dynamically adding groups or singletons of processes
  QGridLayout* add_category_of_processes(std::string);
  QGridLayout* add_single_process(std::string);

  // utility methods
  bool IsOnline() { return (!online_nodes_.empty()); }
  void Quit() { QCoreApplication::quit(); }

 public slots:
  void Process();
  void StartAllCheckedCb();
  void StopAllCheckedCb();
  void EnableScripts(bool);
  void EnableVariables(bool);
  void UpdateConfiguration();

 private:
  void closeEvent(QCloseEvent*);

  std::string                                 dispatcher_lock_file_path_;
  std::shared_ptr<dispatcher::DispatcherNode> ros_node_;
  rclcpp::executors::SingleThreadedExecutor   ros_executor_;

  std::shared_ptr<rclcpp::node_interfaces::NodeGraph> node_graph_;
  std::vector<std::pair<std::string, std::string>>    online_nodes_;

  QTimer*      timer_                          = nullptr;
  QGroupBox*   groupbox_main_                  = nullptr;
  QVBoxLayout* vlayout_main_                   = nullptr;
  QComboBox*   configuration_combo_box_        = nullptr;
  QSplitter*   splitter_of_groupboxes_         = nullptr;
  QGroupBox*   groupbox_processes_             = nullptr;
  QVBoxLayout* layout_groupboxes_of_processes_ = nullptr;
  std::unordered_map<std::string, QGridLayout*> map_grid_layouts_;
  QGroupBox*                                    script_group_box_   = nullptr;
  QGridLayout*                                  script_layout_      = nullptr;
  QGroupBox*                                    variable_group_box_ = nullptr;
  QGridLayout*                                  variable_layout_    = nullptr;

  QSize minimumSizeHint() const { return QSize(30, 30); }
  QSize sizeHint() const { return QSize(30, 30); }
  void  InitializeLayout();
  void  PopulateLayout();

 signals:
};

}  // namespace dispatcher

#endif
