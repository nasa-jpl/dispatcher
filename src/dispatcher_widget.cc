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
  // Set these type of gb to be borderless
  new_gb->setObjectName(QString("myGb"));
  new_gb->setStyleSheet(QString("QGroupBox#myGb {border: 0}"));

  return new_gb;
}

typedef enum {
  SPACER_TYPE_FIXED_HEIGHT = 0,
  SPACER_TYPE_VERT_PILLOW,
  SPACER_TYPE_HORIZ_PILLOW,
} DispatcherSpacerItemType;

static QSpacerItem* DispatcherSpacerItem(DispatcherSpacerItemType type)
{
  QSpacerItem* new_spacer = nullptr;
  switch (type) {
    case SPACER_TYPE_FIXED_HEIGHT:
      // Spacer with fixed height and expands to consume all width
      new_spacer =
          new QSpacerItem(0, 16, QSizePolicy::Expanding, QSizePolicy::Minimum);
      break;
    case SPACER_TYPE_VERT_PILLOW:
      // Pillow spacer because this expands to fill up all vertical space
      new_spacer =
          new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
      break;
    case SPACER_TYPE_HORIZ_PILLOW:
      // Pillow spacer because this expands to fill up all horizontal space
      new_spacer =
          new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
      break;
    default:
      // By default, return a fixed sized spacer
      new_spacer = new QSpacerItem(16, 16);
      break;
  }

  assert(new_spacer);
  return new_spacer;
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
  if (settings.contains("geometry")) {
    EVR_ACTIVITY_HI_REF(ros_node_,
                        "Restoring window size from previous session...");
    restoreGeometry(settings.value("geometry").toByteArray());
  } else {
    EVR_ACTIVITY_HI_REF(ros_node_,
                        "Unable to restore window size from previous session "
                        "so defaulting to 520x600.");
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

  // Add a fixed height spacer
  vlayout_main_->insertSpacerItem(
      1, DispatcherSpacerItem(SPACER_TYPE_FIXED_HEIGHT));

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
  layout_groupboxes_of_processes_->setContentsMargins(0, 0, 0, 0);
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
  QGridLayout* layout_ss_all = add_single_process("start_stop_all");
  QPushButton* start         = new QPushButton("start all checked", this);
  start->setStyleSheet(QString("color: green"));
  layout_ss_all->addWidget(start, 1, 2);
  connect(start, SIGNAL(clicked()), this, SLOT(StartAllCheckedCb()));

  QPushButton* stop = new QPushButton("stop all checked", this);
  stop->setStyleSheet(QString("color: red"));
  layout_ss_all->addWidget(stop, 1, 3);
  connect(stop, SIGNAL(clicked()), this, SLOT(StopAllCheckedCb()));

  // Add spacers to this start_stop_all GB to position things nicely
  layout_ss_all->addItem(DispatcherSpacerItem(SPACER_TYPE_FIXED_HEIGHT), 0, 0);
  layout_ss_all->addItem(DispatcherSpacerItem(SPACER_TYPE_HORIZ_PILLOW), 1, 4);
  layout_ss_all->addItem(DispatcherSpacerItem(SPACER_TYPE_HORIZ_PILLOW), 1, 0);
  layout_ss_all->addItem(DispatcherSpacerItem(SPACER_TYPE_VERT_PILLOW), 2, 0);

  // Add spacers to ScriptItem buttons to position things nicely
  script_layout_->addItem(DispatcherSpacerItem(SPACER_TYPE_VERT_PILLOW), 0, 0);
  script_layout_->addItem(DispatcherSpacerItem(SPACER_TYPE_VERT_PILLOW),
                          script_layout_->rowCount(), 0);
  variable_layout_->addItem(DispatcherSpacerItem(SPACER_TYPE_VERT_PILLOW), 0,
                            0);
  variable_layout_->addItem(DispatcherSpacerItem(SPACER_TYPE_VERT_PILLOW),
                            variable_layout_->rowCount(), 0);

  // Must do this towards the end
  // Establish that DispatcherWidget is parent of groupbox_main_, which
  // contains the layouts of all the QWidgets we have initialized and
  // populated so far
  groupbox_main_->setLayout(vlayout_main_);
  setWidget(groupbox_main_);
  // Forces Widgets inside QScrollArea to scale with QScrollArea
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
  v_layout_gb_single->setContentsMargins(-1, 0, -1, 0);
  QGroupBox*   gb_single_child          = DispatcherGroupBox(gb_single);
  QGridLayout* g_layout_gb_single_child = new QGridLayout(gb_single_child);
  g_layout_gb_single_child->setContentsMargins(-1, 0, -1, 0);
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
  vec_collapsible_widgets_.push_back(widget);

  map_grid_layouts_[category_name] = widget->get_grid_layout();
  return map_grid_layouts_[category_name];
}

dispatcher::DispatcherCategoryWidget::DispatcherCategoryWidget(
    QWidget* parent, std::string category_name)
    : QGroupBox(parent)
{
  this->setObjectName(QString("categoryGb"));
  this->setStyleSheet(QString("QGroupBox#categoryGb {border:0}"));
  this->setContentsMargins(0, 0, 0, 0);
  QVBoxLayout* v_layout_category = new QVBoxLayout(parent);
  v_layout_category->setContentsMargins(-1, 0, -1, 0);

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
  v_layout_category->addWidget(toggle_groupbox_);

  grid_layout_ = new QGridLayout(toggle_groupbox_);
  // These ContentMargins were hand-picked to ensure vertical alignment between
  // the buttons within a collapsible category and those that are not
  grid_layout_->setContentsMargins(6, 4, 6, 4);
  toggle_groupbox_->setLayout(grid_layout_);
  this->setLayout(v_layout_category);

  animation_ = new QPropertyAnimation(toggle_groupbox_, "maximumHeight");
  animation_->setDuration(200);
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