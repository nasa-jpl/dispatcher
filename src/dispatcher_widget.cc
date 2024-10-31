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

/*!
@brief class constructor for DispatcherWidget application
*/
dispatcher::DispatcherWidget::DispatcherWidget(
    QWidget* parent, std::string dispatcher_lock_file_path)
    : QWidget(parent)
{
  CheckTmuxExistsThrowException();

  dispatcher_lock_file_path_ = dispatcher_lock_file_path;
  CheckLockFileExistsThrowException(dispatcher_lock_file_path_);

  QVBoxLayout* layout_ = new QVBoxLayout(parent);
  splitter             = new QSplitter(parent);
  splitter->setOrientation(Qt::Vertical);
  splitter->setStyleSheet(
      "QSplitter::handle:vertical { background-color: darkGray; width: "
      "5px; margin-top: 2px; margin-bottom: 2px; }");

  // combo box
  configuration_combo_box_ = new QComboBox();
  layout_->addWidget(configuration_combo_box_);

  // upper group box / grid
  QGroupBox* top_split = new QGroupBox();
  QGroupBox* upper_box = new QGroupBox();
  // upper_box->setStyleSheet(QString("QGroupBox {border:0}"));
  upper_box->setContentsMargins(QMargins(-1, -1, -1, 0));

  toggleButton = new QToolButton();
  toggleButton->setStyleSheet("QToolButton { border: none; }");
  toggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  toggleButton->setArrowType(Qt::ArrowType::RightArrow);
  toggleButton->setText("My Section");
  toggleButton->setCheckable(true);
  toggleButton->setChecked(false);

  scrollArea = new QScrollArea();
  scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  // scrollArea->setFixedSize(480, 200);
  scrollArea->setMaximumHeight(0);   // Initially collapsed
  scrollArea->setMinimumHeight(0);   // Initially collapsed
  scrollArea->setMinimumWidth(480);  // Initially collapsed
  // scrollArea->setStyleSheet(QString("QScrollArea {border:0}"));
  scrollArea->setFrameShape(QFrame::NoFrame);
  // scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  grid_layout_ = new QGridLayout(upper_box);
  // grid_layout_->setSizeConstraint(QLayout::SetFixedSize);
  // layout_->addWidget(toggleButton);
  // layout_->addWidget(scrollArea);
  QVBoxLayout* top_split_layout = new QVBoxLayout(top_split);
  top_split_layout->addWidget(toggleButton);
  top_split_layout->addWidget(scrollArea);
  splitter->addWidget(top_split);

  animation = new QPropertyAnimation(scrollArea, "maximumHeight");
  animation->setDuration(300);
  connect(toggleButton, &QToolButton::toggled, this, &DispatcherWidget::toggle);

  // lower group box / grid
  script_group_box_ = new QGroupBox();
  script_group_box_->setStyleSheet(QString("QGroupBox {border:0}"));
  script_group_box_->setContentsMargins(QMargins(-1, -1, -1, 0));
  script_layout_ = new QGridLayout(script_group_box_);
  // layout_->addWidget(script_group_box_);
  splitter->addWidget(script_group_box_);

  variable_group_box_ = new QGroupBox();
  variable_group_box_->setStyleSheet(QString("QGroupBox {border:0}"));
  variable_group_box_->setContentsMargins(QMargins(-1, -1, -1, 0));
  variable_layout_ = new QGridLayout(variable_group_box_);
  // layout_->addWidget(variable_group_box_);
  splitter->addWidget(variable_group_box_);

  layout_->addWidget(splitter);

  ros_node_ = std::make_shared<dispatcher::DispatcherNode>(this);

  int          index = grid_layout_->rowCount();
  QPushButton* start = new QPushButton("start all checked", this);
  start->setStyleSheet(QString("color: green"));
  start->setFlat(false);
  grid_layout_->addWidget(start, index, 2);
  connect(start, SIGNAL(clicked()), this, SLOT(StartAllCheckedCb()));

  QPushButton* stop = new QPushButton("stop all checked", this);
  stop->setStyleSheet(QString("color: red"));
  grid_layout_->addWidget(stop, index, 3);
  connect(stop, SIGNAL(clicked()), this, SLOT(StopAllCheckedCb()));

  QSpacerItem* spacer =
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  grid_layout_->addItem(spacer, index, 4);

  QSpacerItem* spacer2 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  grid_layout_->addItem(spacer2, grid_layout_->rowCount(), 0);

  double loop_period_ms = 1.0 / ros_node_->GetTimerRate() * 1000.0;
  EVR_DIAGNOSTIC_REF(ros_node_, "Using loop period %f ms", loop_period_ms);

  timer_ = new QTimer;
  connect(timer_, SIGNAL(timeout()), this, SLOT(Process()));
  timer_->start(loop_period_ms);

  scrollArea->setWidget(upper_box);
  // layout_->setSizeConstraint(QLayout::SetFixedSize);
  layout_->setSpacing(0);
  setLayout(layout_);

  connect(configuration_combo_box_, SIGNAL(currentTextChanged(QString)), this,
          SLOT(UpdateConfiguration()));
}

void dispatcher::DispatcherWidget::toggle(bool checked)
{
  toggleButton->setArrowType(checked ? Qt::ArrowType::DownArrow
                                     : Qt::ArrowType::RightArrow);

  animation->setStartValue(scrollArea->maximumHeight());
  animation->setEndValue(checked ? scrollArea->sizeHint().height()
                                 : 0);  // This is the uncollapsed height
  animation->start();
}

/*!
@brief class destructor
*/
dispatcher::DispatcherWidget::~DispatcherWidget()
{
  DeleteLockFile(dispatcher_lock_file_path_);
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
