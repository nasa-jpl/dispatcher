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

static QGroupBox* DispatcherGroupBox(QWidget* parent)
{
  QGroupBox* new_gb = new QGroupBox(parent);
  new_gb->setStyleSheet("QGroupBox {border: 0px; }");
  new_gb->setContentsMargins(0, 0, 0, 0);
  return new_gb;
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
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  CheckTmuxExistsThrowException();
  dispatcher_lock_file_path_ = dispatcher_lock_file_path;
  CheckLockFileExistsThrowException(dispatcher_lock_file_path_);

  // Sets up almost all of the QWidgets used in the Dispatcher GUI
  InitializeWidgets();

  // Reads the dispatcher.yaml and setup/populates QWidgets with more
  // information (i.e processes, configurations)
  ros_node_ = std::make_shared<dispatcher::DispatcherNode>(this);

  // Pick up WindowSize settings, if avail, from last session to restore
  QSettings settings;
  if (settings.status() == QSettings::NoError) {
    EVR_ACTIVITY_HI_REF(ros_node_,
                        "Restoring window size from previous session...");
    restoreGeometry(settings.value("geometry").toByteArray());
  } else {
    EVR_ACTIVITY_HI_REF(ros_node_,
                        "Unable to restore window size from previous session "
                        "so will default to something arbitrary");
    resize(520, 600);
  }

  // Now that QWidgets are properly populated, do some finishing touches (i.e
  // start/stop_all, connect signals to callbacks
  FinalizeWidgets();
}

/*!
@brief class destructor
*/
dispatcher::DispatcherWidget::~DispatcherWidget()
{
  DeleteLockFile(dispatcher_lock_file_path_);
}

void dispatcher::DispatcherWidget::InitializeWidgets()
{
  // Groupbox that contains everything, child of DispatcherWidget
  groupbox_main_ = DispatcherGroupBox(this);
  vlayout_main_  = new QVBoxLayout(groupbox_main_);

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

  // ProcessItem GroupBox, will contain Groupboxes of Items
  groupbox_processes_             = DispatcherGroupBox(splitter_of_groupboxes_);
  layout_groupboxes_of_processes_ = new QVBoxLayout(groupbox_processes_);
  groupbox_processes_->setLayout(layout_groupboxes_of_processes_);
  splitter_of_groupboxes_->addWidget(groupbox_processes_);

  // ScriptItem Groupbox
  script_group_box_ = DispatcherGroupBox(splitter_of_groupboxes_);
  script_layout_    = new QGridLayout(script_group_box_);
  splitter_of_groupboxes_->addWidget(script_group_box_);

  // Variables Groupbox
  variable_group_box_ = DispatcherGroupBox(splitter_of_groupboxes_);
  variable_layout_    = new QGridLayout(variable_group_box_);
  splitter_of_groupboxes_->addWidget(variable_group_box_);
}

void dispatcher::DispatcherWidget::FinalizeWidgets()
{
  // Induce a change in configuration upon user selection
  connect(configuration_combo_box_, SIGNAL(currentTextChanged(QString)), this,
          SLOT(UpdateConfiguration()));

  // Create a GroupBox to hold start/stop_all buttons
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

  // Spacers to flank newly minted buttons
  QSpacerItem* spacer =
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  layout->addItem(spacer, index, 4);

  QSpacerItem* spacer3 =
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  layout->addItem(spacer3, index, 0);

  // Spacers below the new buttons
  QSpacerItem* spacer2 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  layout->addItem(spacer2, layout->rowCount(), 0);

  // Must do this towards the end, but establish that DispatcherWidget is a
  // parent of groupbox_main_, which contains all the QWidgets we have
  // initialized and populated so far
  vlayout_main_->setSpacing(0);
  groupbox_main_->setLayout(vlayout_main_);
  setWidget(groupbox_main_);
  // Forces Widgets inside QScrollArea to scale with QScaleArea
  setWidgetResizable(true);

  // Set up timer
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
  EVR_ACTIVITY_HI_REF(ros_node_, "Shutting down after saving window state.");
  QSettings settings;
  settings.setValue("geometry", saveGeometry());

  ros_node_->StopAll();
  rclcpp::shutdown();
  QCoreApplication::quit();
}

QGridLayout* dispatcher::DispatcherWidget::add_single_process(std::string name)
{
  QGroupBox*   gb_single          = DispatcherGroupBox(groupbox_processes_);
  QVBoxLayout* v_layout_gb_single = new QVBoxLayout(gb_single);
  QGroupBox*   gb_single_child    = DispatcherGroupBox(gb_single);
  QGridLayout* g_layout_gb_single_child = new QGridLayout(gb_single_child);
  gb_single_child->setLayout(g_layout_gb_single_child);
  gb_single->setLayout(v_layout_gb_single);
  v_layout_gb_single->addWidget(gb_single_child);
  layout_groupboxes_of_processes_->addWidget(gb_single);

  map_grid_layouts_[name] = g_layout_gb_single_child;
  return map_grid_layouts_[name];
}

QGridLayout* dispatcher::DispatcherWidget::add_category_of_processes(
    std::string category_name)
{
  dispatcher::DispatcherCategoryWidget* widget =
      new dispatcher::DispatcherCategoryWidget(groupbox_processes_,
                                               category_name);
  layout_groupboxes_of_processes_->addWidget(widget);

  map_grid_layouts_[category_name] = widget->get_grid_layout();
  return map_grid_layouts_[category_name];
}

dispatcher::DispatcherCategoryWidget::DispatcherCategoryWidget(
    QWidget* parent, std::string category_name)
    : QGroupBox(parent)
{
  this->setStyleSheet(QString("QGroupBox {border:0}"));
  this->setContentsMargins(QMargins(-1, -1, -1, 0));
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
  toggle_groupbox_ = DispatcherGroupBox(this);
  toggle_groupbox_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  toggle_groupbox_->setMaximumHeight(0);  // Initially collapsed
  toggle_groupbox_->setMinimumHeight(0);  // Initially collapsed
  v_layout_category->addWidget(toggle_groupbox_);

  grid_layout_ = new QGridLayout(toggle_groupbox_);
  toggle_groupbox_->setLayout(grid_layout_);
  this->setLayout(v_layout_category);

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