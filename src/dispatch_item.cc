#include "dispatcher/dispatch_item.h"
#include "dispatcher/dispatcher_widget.h"

#include <stdlib.h>

#include <string>

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QPixmap>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include <yaml-cpp/yaml.h>

static bool check_gnome_terminal_exists()
{
  return (system("which gnome-terminal") == 0) ? true : false;
}

/*!
@brief class constructor for DispatchItem
*/
dispatcher::DispatchItem::DispatchItem(QWidget*                    parent,
                                       dispatcher::DispatcherNode* ros_node,
                                       const YAML::Node&           node)
    : QWidget(parent)
{
  ros_node_          = ros_node;
  name_              = node["name"].as<std::string>();
  node_namespace_    = node["namespace"].as<std::string>();
  node_name_         = node["node_name"].as<std::string>();
  cmd_               = node["cmd"].as<std::string>();
  bool start_checked = node["start_checked"].as<bool>();

#if 0
  // TODO @kwehage subscribe to Health topics 
  // using std_msgs::msg::Empty or diagnostic_msgs::msg::DiagnosticStatus
  // message types
  //
  if (node["topics"]) {
    for (auto &topic : node["topics"]) {
      CFW_INFO("Subscribing to topic: %s", topic.as<std::string>().c_str());
    }
  }
#endif

  dispatcher_         = dynamic_cast<dispatcher::DispatcherWidget*>(parent);
  QGridLayout* layout = dispatcher_->get_layout();
  assert(layout);
  index_ = layout->rowCount();

  red_status_icon_ =
      QPixmap::fromImage(QImage(":/icons/red.png").scaledToHeight(20));
  green_status_icon_ =
      QPixmap::fromImage(QImage(":/icons/green.png").scaledToHeight(20));

  label_ = new QLabel("");
  label_->setPixmap(red_status_icon_);
  layout->addWidget(label_, index_, 1);

  check_box_                 = new QCheckBox(QString(name_.c_str()), this);
  Qt::CheckState check_state = start_checked ? Qt::Checked : Qt::Unchecked;
  check_box_->setChecked(check_state);
  layout->addWidget(check_box_, index_, 0);

  QPushButton* start = new QPushButton("start", this);
  start->setStyleSheet(QString("color: green"));
  layout->addWidget(start, index_, 2);
  connect(start, SIGNAL(clicked()), this, SLOT(StartCb()));

  QPushButton* stop = new QPushButton("stop", this);
  stop->setStyleSheet(QString("color: red"));
  layout->addWidget(stop, index_, 3);
  connect(stop, SIGNAL(clicked()), this, SLOT(StopCb()));

  QPushButton* terminal = new QPushButton(this);
  terminal->setIcon(QIcon(":/icons/terminal.png"));
  terminal->setIconSize(QSize(20, 20));
  layout->addWidget(terminal, index_, 4);
  connect(terminal, SIGNAL(clicked()), this, SLOT(TerminalCb()));

  QSpacerItem* spacer =
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  layout->addItem(spacer, index_, 5);
}

bool dispatcher::DispatchItem::is_checked()
{
  if (check_box_->checkState() == 0) {
    // unchecked case
    return false;
  } else {
    // partially and fully checked cases
    return true;
  }
}

/*!
@brief class destructor
*/
dispatcher::DispatchItem::~DispatchItem() {}

void dispatcher::DispatchItem::Process()
{
  const auto& online_nodes_ = ros_node_->get_online_nodes();

  bool previous_state = online_;
  online_             = false;
  for (auto& online_node : online_nodes_) {
    if (node_name_ == online_node.first &&
        node_namespace_ == online_node.second) {
      online_ = true;
    }
  }
  if (online_ != previous_state) {
    if (online_) {
      label_->setPixmap(green_status_icon_);
    } else {
      label_->setPixmap(red_status_icon_);
    }
  }
}

void dispatcher::DispatchItem::StartCb()
{
  if (online_) {
    CFW_WARN(
        "Refusing to start node %s in tmux session: %s "
        "because node was detected as already running on ROS2 message bus",
        node_name_.c_str(), name_.c_str());
    return;
  }

  CFW_INFO("Starting node: %s in tmux session: %s", node_name_.c_str(),
           name_.c_str());
  CFW_INFO("system cmd: %s", cmd_.c_str());

  // kill the tmux session if it exists, flushes out old
  // sessions that might have processes still running in them
  //(void)TmuxKillSession();

  // generate a new tmux session using 'name' key in yaml config
  //(void)TmuxNewSession();

  // cd to workspace directory
  TmuxSendKeys("cd" + ros_node_->get_workspace());

  // source ros environment
  TmuxSendKeys("source install/setup.bash");

  // run user-supplied command, you can call an executable directly, start a
  // node or launch script
  TmuxSendKeys(cmd_);
}

void dispatcher::DispatchItem::StopCb()
{
  if (online_) {
    CFW_INFO("Stopping node: %s in tmux session: %s", node_name_.c_str(),
             name_.c_str());
    TmuxSendKeys("C-C");
  }
  //(void)TmuxKillSession();
}

void dispatcher::DispatchItem::TerminalCb()
{
  if (!check_gnome_terminal_exists()) {
    CFW_WARN(
        "Cannot attach to tmux session because 'gnome-terminal' does not exist "
        "on this "
        "system; you can attach to the session by running `tmux a -t %s` "
        "from any terminal",
        name_.c_str());
    return;
  }
  if (!online_) {

    CFW_WARN( "Attempting to attach to offline tmux session: %s", name_.c_str());

    // kill the tmux session if it exists, flushes out old
    // sessions that might have processes still running in them
    //(void)TmuxKillSession(); 

    // generate a new tmux session using 'name' key in yaml config
    //(void)TmuxNewSession();
  }

  // Start a gnome session and attach a tmux session
  SystemCall("gnome-terminal -- tmux a -t " + name_);
}

bool dispatcher::DispatchItem::TmuxKillSession(){

  std::string cmd = "tmux kill-session -t " + name_;
  int result = SystemCall(cmd);
  if(result == 1){
    CFW_WARN("tmux session %s could not be killed", name_.c_str());
    return false;
  }
  return true;
}

bool dispatcher::DispatchItem::TmuxNewSession(){

  std::string cmd = "tmux new -d -s " + name_;
  int result = SystemCall(cmd);
  if (result == 1) {
    CFW_WARN("tmux session %s could not be created", name_.c_str());
    return false;
  }
  return true;
}

void dispatcher::DispatchItem::TmuxSendKeys(std::string cmd_str){

  std::string cmd = "tmux send-keys -t " + name_ + ":0 '" + cmd_str + "' Enter";
  (void)system(cmd.c_str());
}

int dispatcher::DispatchItem::SystemCall(std::string cmd){
  CFW_DEBUG("Issuing subprocess call `%s`", cmd.c_str());
  int result = system(cmd.c_str());
  CFW_DEBUG("Subprocess call `%s` returned result %d", cmd.c_str(), result); 
  return result;
}
