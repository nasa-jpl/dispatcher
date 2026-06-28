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
#include <QSettings>
#include <QSocketNotifier>
#include <QSplitter>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

#include <rclcpp/node_interfaces/node_graph.hpp>
#include <rclcpp/rclcpp.hpp>

#include <sensor_msgs/msg/joint_state.hpp>

namespace dispatcher
{
/*! @brief Returns the build version string for the dispatcher package. */
std::string version();
/*! @brief Returns the source-control branch string for the dispatcher package. */
std::string branch();
/*! @brief Returns the source-control commit string for the dispatcher package. */
std::string commit();

/*! @brief Exception type used for fatal dispatcher widget startup errors. */
class DispatcherException : public std::exception
{
 private:
  const char* message_;

 public:
  explicit DispatcherException(const char* message) { message_ = message; };
  /*! @brief Returns the stored exception message. */
  const char* what() const noexcept override { return message_; }
};

/*! @brief Collapsible container used to group related process items in the UI. */
class DispatcherCategoryWidget : public QGroupBox
{
  Q_OBJECT  // must be included to add qt meta information

 public:
  /*! @brief Constructs a collapsible process category. */
  explicit DispatcherCategoryWidget(
      QWidget* parent = 0, const std::string& category_name = "Default");
  /*! @brief Destroys the category widget. */
  ~DispatcherCategoryWidget();

  /*! @brief Returns whether the category is currently expanded. */
  bool get_checked_state() { return toggle_button_->isChecked(); }
  /*! @brief Expands or collapses the category widget. */
  void set_checked_state(bool check) { toggle_button_->setChecked(check); }
  /*! @brief Returns the layout used to insert child process widgets. */
  QGridLayout* get_grid_layout() { return grid_layout_; }

 public slots:
  /*! @brief Animates the category open or closed. */
  void ToggleCb(bool);

 private:
  QGridLayout*        grid_layout_     = nullptr;
  QToolButton*        toggle_button_   = nullptr;
  QGroupBox*          toggle_groupbox_ = nullptr;
  QPropertyAnimation* animation_       = nullptr;
};

/*! @brief Main scrollable Qt widget that renders and controls the dispatcher UI. */
class DispatcherWidget : public QScrollArea
{
  Q_OBJECT  // must be included to add qt meta information

 public:
  /*! @brief Constructs the main dispatcher widget and acquires the lock file. */
  explicit DispatcherWidget(QWidget* parent = 0,
                            const std::string& dispatcher_lock_file_path =
                                "/tmp/dispatcher.lock");
  /*! @brief Destroys the widget and releases the lock file. */
  ~DispatcherWidget();

  /*! @brief Returns the layout used for script-button widgets. */
  QGridLayout* get_script_layout() { return script_layout_; }
  /*! @brief Returns the layout used for variable selector widgets. */
  QGridLayout* get_variable_layout() { return variable_layout_; }
  /*! @brief Returns the backing ROS node used by the widget. */
  std::shared_ptr<dispatcher::DispatcherNode> get_ros_node()
  {
    return ros_node_;
  }
  /*! @brief Returns the ROS graph snapshot cached by the widget. */
  const std::vector<std::pair<std::string, std::string>>& get_online_nodes()
  {
    return online_nodes_;
  }
  /*! @brief Returns the configuration combo box widget. */
  QComboBox*  get_configuration_combo_box() { return configuration_combo_box_; }
  /*! @brief Returns the currently selected configuration name. */
  std::string get_current_configuration()
  {
    return configuration_combo_box_->currentText().toStdString();
  }
  /*! @brief Returns all collapsible category widgets currently in the UI. */
  std::vector<DispatcherCategoryWidget*> get_collapsible_widgets()
  {
    return vec_collapsible_widgets_;
  }
  /*! @brief Returns all collapsible script category widgets currently in the UI. */
  std::vector<DispatcherCategoryWidget*> get_script_collapsible_widgets()
  {
    return vec_script_collapsible_widgets_;
  }

  /*! @brief Creates a collapsible category section and returns its child layout. */
  QGridLayout* AddCategoryOfProcesses(const std::string&);
  /*! @brief Creates a single-process section and returns its child layout. */
  QGridLayout* AddSingleProcess(const std::string&);
  /*! @brief Creates a collapsible category section for scripts and returns its child layout. */
  QGridLayout* AddCategoryOfScripts(const std::string&);

  /*! @brief Returns true when at least one ROS node is reported online. */
  bool IsOnline() { return (!online_nodes_.empty()); }
  /*! @brief Requests Qt application shutdown. */
  void Quit() { QCoreApplication::quit(); }

 public slots:
  /*! @brief Advances one UI/ROS processing iteration. */
  void Process();
  /*! @brief Starts all checked process items. */
  void StartAllCheckedCb();
  /*! @brief Stops all checked process items. */
  void StopAllCheckedCb();
  /*! @brief Shows or hides the script-button panel. */
  void EnableScripts(bool);
  /*! @brief Shows or hides the variable-selection panel. */
  void EnableVariables(bool);
  /*! @brief Applies the currently selected configuration to the widget. */
  void UpdateConfiguration();

 private:
  void InitializeWidgets();
  void FinalizeWidgets();
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
  std::vector<DispatcherCategoryWidget*>        vec_collapsible_widgets_;
  QGroupBox*                                    script_group_box_   = nullptr;
  QGridLayout*                                  script_layout_      = nullptr;
  QVBoxLayout*                                  script_main_layout_ = nullptr;
  std::vector<DispatcherCategoryWidget*>        vec_script_collapsible_widgets_;
  QGroupBox*                                    variable_group_box_ = nullptr;
  QGridLayout*                                  variable_layout_    = nullptr;

  QSize minimumSizeHint() const { return QSize(30, 30); }
  QSize sizeHint() const { return QSize(30, 30); }

 signals:
};

}  // namespace dispatcher

#endif
