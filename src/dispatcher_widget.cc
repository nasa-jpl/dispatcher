#include "dispatcher/dispatcher_widget.h"
#include "dispatcher/dispatcher_node.h"

#include "dispatcher/config.h"

#include <stdio.h>

#include <fstream>
#include <iostream>
#include <string>

#include <QCoreApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QSocketNotifier>
#include <QSpacerItem>
#include <QWidget>

#include <rclcpp/node_interfaces/node_base_interface.hpp>

#include <yaml-cpp/yaml.h>

#include <cfw/cfw.h>

static bool CheckTmuxExists()
{
  return (system("which tmux") == 0) ? true : false;
}

static void CheckTmuxExistsThrowException()
{
  if (!CheckTmuxExists()) {
    throw dispatcher::DispatcherException(
        "Program `tmux` not found on this system, terminating...");
  }
}

static void CreateLockFile(const std::string& lock_filename)
{
  std::ofstream output(lock_filename.c_str());
}

static void DeleteLockFile(const std::string& lock_filename)
{
  if (remove(lock_filename.c_str()) != 0) {
    std::string error_message = "Error deleting lock file: " + lock_filename;
    throw dispatcher::DispatcherException(error_message.c_str());
  }
}

static bool CheckLockFileExists(const std::string& lock_filename)
{
  std::ifstream lock_file(lock_filename.c_str());
  return lock_file.good();
}

static void CheckLockFileExistsThrowException(const std::string& lock_filename)
{
  if (CheckLockFileExists(lock_filename)) {
    std::string error_message = "Could not get lock on file " + lock_filename +
                                "; an instance of Dispatcher appears to "
                                "already be running, terminating...";
    throw dispatcher::DispatcherException(error_message.c_str());
  } else {
    CreateLockFile(lock_filename);
  }
}

void dispatcher::DispatcherWidget::EnableScripts(bool enable)
{
  if (script_group_box_ == nullptr) {
    EVR_FATAL_REF(ros_node_, "script_group_box_ was not correctly initialized");
  }
  script_group_box_->setVisible(enable);
}

void dispatcher::DispatcherWidget::EnableVariables(bool enable)
{
  if (variable_group_box_ == nullptr) {
    EVR_FATAL_REF(ros_node_,
                  "variable_group_box_ was not correctly initialized");
  }
  variable_group_box_->setVisible(enable);
}

/*!
@brief class constructor for DispatcherWidget application
*/
dispatcher::DispatcherWidget::DispatcherWidget(
    QWidget* parent, std::string dispatcher_lock_file_path)
    : QScrollArea(parent)
{
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);  // TODO
  CheckTmuxExistsThrowException();

  dispatcher_lock_file_path_ = dispatcher_lock_file_path;
  CheckLockFileExistsThrowException(dispatcher_lock_file_path_);

  InitializeLayout();

  ros_node_ = std::make_shared<dispatcher::DispatcherNode>(this);

  PopulateLayout();
}

/*!
@brief class destructor
*/
dispatcher::DispatcherWidget::~DispatcherWidget()
{
  DeleteLockFile(dispatcher_lock_file_path_);
}

void dispatcher::DispatcherWidget::InitializeLayout()
{
  // Groupbox that contains everything, child of DispatcherWidget
  groupbox_main_ = new QGroupBox(this);
  groupbox_main_->setStyleSheet(QString("QGroupBox {border:0}"));
  groupbox_main_->setContentsMargins(QMargins(-1, -1, -1, 0));
  vlayout_main_ = new QVBoxLayout(groupbox_main_);

  // Combo box to choose between configurations
  configuration_combo_box_ = new QComboBox(groupbox_main_);
  vlayout_main_->addWidget(configuration_combo_box_);

  // Use QSplitter to organize three adjustable sections
  // - Our ROS and Shell processes
  // - ScriptItems
  // - Variables
  splitter_of_groupboxes_ = new QSplitter(groupbox_main_);
  splitter_of_groupboxes_->setOrientation(Qt::Vertical);
  splitter_of_groupboxes_->setStyleSheet(
      "QSplitter::handle:vertical { background-color: darkGray; width: "
      "5px; margin-top: 2px; margin-bottom: 2px; }");
  vlayout_main_->addWidget(splitter_of_groupboxes_);

  // GroupBoxes of Process Items
  // TODO: Just one for now
  groupbox_processes_ = new QGroupBox(splitter_of_groupboxes_);
  groupbox_processes_->setContentsMargins(QMargins(-1, -1, -1, 0));
  layout_groupboxes_of_processes_ = new QVBoxLayout(groupbox_processes_);
  groupbox_processes_->setLayout(layout_groupboxes_of_processes_);

  // toggle_area = new QGroupBox(groupbox_processes);
  // toggle_area->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  // toggle_area->setMaximumHeight(0);  // Initially collapsed
  // toggle_area->setMinimumHeight(0);  // Initially collapsed
  // toggle_area->setMinimumWidth(480);
  // toggle_area->setStyleSheet(QString("QGroupBox {border:0}"));
  // layout_groupbox_processes->addWidget(toggle_area);
  // grid_layout_ = new QGridLayout(toggle_area);
  // toggle_area->setLayout(grid_layout_);
  // // grid_layout_->setSizeConstraint(QLayout::SetFixedSize);

  splitter_of_groupboxes_->addWidget(groupbox_processes_);

  // ScriptItem Groupbox
  script_group_box_ = new QGroupBox(splitter_of_groupboxes_);
  script_group_box_->setStyleSheet(QString("QGroupBox {border:0}"));
  script_group_box_->setContentsMargins(QMargins(-1, -1, -1, 0));
  script_layout_ = new QGridLayout(script_group_box_);
  splitter_of_groupboxes_->addWidget(script_group_box_);

  // Variables Groupbox
  variable_group_box_ = new QGroupBox(splitter_of_groupboxes_);
  variable_group_box_->setStyleSheet(QString("QGroupBox {border:0}"));
  variable_group_box_->setContentsMargins(QMargins(-1, -1, -1, 0));
  variable_layout_ = new QGridLayout(variable_group_box_);
  splitter_of_groupboxes_->addWidget(variable_group_box_);
}

void dispatcher::DispatcherWidget::PopulateLayout()
{
  QGridLayout* layout = add_single_process("start_stop_all");
  int          index  = layout->rowCount();
  QPushButton* start  = new QPushButton("start all checked", this);
  start->setStyleSheet(QString("color: green"));
  start->setFlat(false);
  layout->addWidget(start, index, 2);
  connect(start, SIGNAL(clicked()), this, SLOT(StartAllCheckedCb()));

  QPushButton* stop = new QPushButton("stop all checked", this);
  stop->setStyleSheet(QString("color: red"));
  layout->addWidget(stop, index, 3);
  connect(stop, SIGNAL(clicked()), this, SLOT(StopAllCheckedCb()));

  QSpacerItem* spacer =
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  layout->addItem(spacer, index, 4);

  QSpacerItem* spacer2 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  layout->addItem(spacer2, layout->rowCount(), 0);

  // // layout_->setSizeConstraint(QLayout::SetFixedSize);
  vlayout_main_->setSpacing(0);
  groupbox_main_->setLayout(vlayout_main_);
  setWidget(groupbox_main_);
  setWidgetResizable(true);

  connect(configuration_combo_box_, SIGNAL(currentTextChanged(QString)), this,
          SLOT(UpdateConfiguration()));

  double loop_period_ms = 1.0 / ros_node_->GetTimerRate() * 1000.0;
  EVR_DIAGNOSTIC_REF(ros_node_, "Using loop period %f ms", loop_period_ms);

  timer_ = new QTimer;
  connect(timer_, SIGNAL(timeout()), this, SLOT(Process()));
  timer_->start(loop_period_ms);
}

void dispatcher::DispatcherWidget::UpdateConfiguration()
{
  ros_node_->UpdateConfiguration();
}

void dispatcher::DispatcherWidget::Process()
{
  if (rclcpp::ok()) {
    ros_node_->Process();
    ros_executor_.spin_node_some(ros_node_);
  } else {
    EVR_ACTIVITY_HI_REF(ros_node_, "Shutting down");
    ros_node_->StopAll();
    rclcpp::shutdown();
    QCoreApplication::quit();
  }
}

void dispatcher::DispatcherWidget::StartAllCheckedCb()
{
  ros_node_->StartChecked();
}

void dispatcher::DispatcherWidget::StopAllCheckedCb()
{
  ros_node_->StopChecked();
}

void dispatcher::DispatcherWidget::closeEvent(QCloseEvent*)
{
  EVR_ACTIVITY_HI_REF(ros_node_, "Shutting down");
  ros_node_->StopAll();
  rclcpp::shutdown();
  QCoreApplication::quit();
}

QGridLayout* dispatcher::DispatcherWidget::add_single_process(std::string name)
{
  QGroupBox* gb_single = new QGroupBox(groupbox_processes_);
  gb_single->setStyleSheet(QString("QGroupBox {border:0}"));
  gb_single->setContentsMargins(QMargins(-1, -1, -1, 0));
  QGridLayout* g_layout_gb_single = new QGridLayout(gb_single);
  gb_single->setLayout(g_layout_gb_single);
  layout_groupboxes_of_processes_->addWidget(gb_single);

  map_grid_layouts_[name] = g_layout_gb_single;
  return map_grid_layouts_[name];
}

QGridLayout* dispatcher::DispatcherWidget::add_category_of_processes(
    std::string category_name)
{
  dispatcher::DispatcherCategoryWidget* widget =
      new dispatcher::DispatcherCategoryWidget(groupbox_processes_,
                                               category_name);
  // QGroupBox* gb_category = new QGroupBox(groupbox_processes_);
  widget->setStyleSheet(QString("QGroupBox {border:0}"));
  widget->setContentsMargins(QMargins(-1, -1, -1, 0));
  layout_groupboxes_of_processes_->addWidget(widget);

  map_grid_layouts_[category_name] = widget->get_grid_layout();
  return map_grid_layouts_[category_name];
}

dispatcher::DispatcherCategoryWidget::DispatcherCategoryWidget(
    QWidget* parent, std::string category_name)
    : QGroupBox(parent)
{
  QVBoxLayout* v_layout_category = new QVBoxLayout(parent);

  // Define a QToolButton so we can click to collapse the below gb
  toggle_button_ = new QToolButton(this);
  toggle_button_->setStyleSheet("QToolButton { border: none; }");
  toggle_button_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  toggle_button_->setArrowType(Qt::ArrowType::RightArrow);
  toggle_button_->setText(QString(category_name.c_str()));
  toggle_button_->setCheckable(true);
  toggle_button_->setChecked(false);
  v_layout_category->addWidget(toggle_button_);

  // Define collapsible QGroupBox to hold all the processes
  toggle_groupbox_ = new QGroupBox(this);
  toggle_groupbox_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  toggle_groupbox_->setMaximumHeight(0);  // Initially collapsed
  toggle_groupbox_->setMinimumHeight(0);  // Initially collapsed
  // // gb_processes->setMinimumWidth(480);
  toggle_groupbox_->setStyleSheet("QGroupBox {border:0}");
  v_layout_category->addWidget(toggle_groupbox_);

  grid_layout_ = new QGridLayout(toggle_groupbox_);
  toggle_groupbox_->setLayout(grid_layout_);
  this->setLayout(v_layout_category);
  // // g_layout_category_processes->setSizeConstraint(QLayout::SetFixedSize);

  animation_ = new QPropertyAnimation(toggle_groupbox_, "maximumHeight");
  animation_->setDuration(300);
  connect(toggle_button_, &QToolButton::toggled, this,
          &DispatcherCategoryWidget::ToggleCb);
}

/*!
@brief class destructor
*/
dispatcher::DispatcherCategoryWidget::~DispatcherCategoryWidget() {}

void dispatcher::DispatcherCategoryWidget::ToggleCb(bool checked)
{
  toggle_button_->setArrowType(checked ? Qt::ArrowType::DownArrow
                                       : Qt::ArrowType::RightArrow);

  animation_->setStartValue(toggle_groupbox_->maximumHeight());
  animation_->setEndValue(checked ? toggle_groupbox_->sizeHint().height()
                                  : 0);  // This is the uncollapsed height
  animation_->start();
}