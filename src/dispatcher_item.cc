#include "dispatcher/dispatcher_item.h"
#include "dispatcher/dispatcher_widget.h"

#include <stdlib.h>

#include <algorithm>
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
  return (system("which gnome-terminal > /dev/null 2>&1") == 0) ? true : false;
}

/*!
@brief class constructor for DispatcherItem
*/
dispatcher::DispatcherItem::DispatcherItem(QWidget*                    parent,
                                           dispatcher::DispatcherNode* ros_node,
                                           const YAML::Node&           node)
    : QWidget(parent)
{
  ros_node_           = ros_node;
  dispatcher_         = dynamic_cast<dispatcher::DispatcherWidget*>(parent);
  QGridLayout* layout = dispatcher_->get_grid_layout();
  assert(layout);
  name_ = node["name"].as<std::string>();
  EVR_ACTIVITY_LO_REF(ros_node_, "Loading properties for node '%s'",
                      name_.c_str());

  if (node["use_environment_variables"]) {
    use_environment_variables_ = node["use_environment_variables"].as<bool>();
  }

  if (node["namespace"] && node["node_name"]) {
    RosNodeMonitorConfig monitor_config;
    monitor_config.namespace_ = node["namespace"].as<std::string>();
    monitor_config.name       = node["node_name"].as<std::string>();
    ros_nodes_.push_back(monitor_config);
  }
  if (node["ros_nodes"]) {
    for (const auto& ros_node : node["ros_nodes"]) {
      RosNodeMonitorConfig monitor_config;
      monitor_config.namespace_ = ros_node["namespace"].as<std::string>();
      monitor_config.name       = ros_node["name"].as<std::string>();
      ros_nodes_.push_back(monitor_config);
    }
  }
  if (node["attach_on_start"]) {
    attach_on_start_ = node["attach_on_start"].as<bool>();
  }

  if (node["cmd"] && node["configurations"]) {
    EVR_FATAL_REF(ros_node_,
                  "'cmd' and 'configurations' keys cannot be used together");
    rclcpp::shutdown();
  }
  if (node["cmd"]) {
    // user has specified a command at root level which applies to all
    // configurations, so create a default 'all' configuration
    DispatcherItemConfig config;
    config.configuration_name = "all";
    config.cmd                = node["cmd"].as<std::string>();

    if (node["hostname"]) {
      config.hostname = node["hostname"].as<std::string>();
    } else {
      config.hostname = "localhost";
    }

    if (node["user"]) {
      config.user = node["user"].as<std::string>();
    }

    configurations_[config.configuration_name] = config;
  }

  if (node["configurations"]) {
    for (const auto& configuration : node["configurations"]) {
      DispatcherItemConfig config;
      config.configuration_name = configuration["name"].as<std::string>();
      config.cmd                = configuration["cmd"].as<std::string>();
      if (configuration["hostname"]) {
        config.hostname = configuration["hostname"].as<std::string>();
      } else {
        config.hostname = "localhost";
      }
      if (configuration["user"]) {
        config.user = configuration["user"].as<std::string>();
      }

      if (configuration["ros_nodes"]) {
        for (const auto& ros_node : configuration["ros_nodes"]) {
          RosNodeMonitorConfig monitor_config;
          monitor_config.namespace_ = ros_node["namespace"].as<std::string>();
          monitor_config.name       = ros_node["name"].as<std::string>();
          config.ros_nodes.push_back(monitor_config);
        }
      }

      configurations_[config.configuration_name] = config;
    }
  }

  bool start_checked = node["start_checked"].as<bool>();
  use_cmd_prefix_    = true;
  if (node["use_cmd_prefix"]) {
    use_cmd_prefix_ = node["use_cmd_prefix"].as<bool>();
  }
  index_ = layout->rowCount();

  // Convert any spaces to underscores in YAML Name parameter
  std::string rep_str = node["name"].as<std::string>();
  std::replace(rep_str.begin(), rep_str.end(), ' ', '_');
  tmux_name_ = std::to_string(index_) + "_" + rep_str;

  if (node["stop_tmux_cmd"]) {
    stop_tmux_cmd_ = node["stop_tmux_cmd"].as<std::string>();
  } else {
    stop_tmux_cmd_ = std::string("C-C");
  }

  grey_status_icon_ =
      QPixmap::fromImage(QImage(":/icons/grey.png").scaledToHeight(20));
  red_status_icon_ =
      QPixmap::fromImage(QImage(":/icons/red.png").scaledToHeight(20));
  green_status_icon_ =
      QPixmap::fromImage(QImage(":/icons/green.png").scaledToHeight(20));
  orange_status_icon_ =
      QPixmap::fromImage(QImage(":/icons/orange.png").scaledToHeight(20));

  check_box_                 = new QCheckBox(QString(name_.c_str()), this);
  Qt::CheckState check_state = start_checked ? Qt::Checked : Qt::Unchecked;
  layout->addWidget(check_box_, index_, 0);
  check_box_->setChecked(check_state);
  check_box_->setAttribute(Qt::WA_TransparentForMouseEvents, false);
  check_box_->setFocusPolicy(Qt::StrongFocus);

  label_ = new QLabel("");
  label_->setPixmap(red_status_icon_);
  layout->addWidget(label_, index_, 1);

  start_ = new QPushButton("start", this);
  start_->setStyleSheet(QString("color: green"));
  layout->addWidget(start_, index_, 2);
  connect(start_, SIGNAL(clicked()), this, SLOT(StartCb()));

  stop_ = new QPushButton("stop", this);
  stop_->setStyleSheet(QString("color: red"));
  layout->addWidget(stop_, index_, 3);
  connect(stop_, SIGNAL(clicked()), this, SLOT(StopCb()));

  terminal_ = new QPushButton(this);
  terminal_->setIcon(QIcon(":/icons/terminal.png"));
  terminal_->setIconSize(QSize(20, 20));
  layout->addWidget(terminal_, index_, 4);
  connect(terminal_, SIGNAL(clicked()), this, SLOT(TerminalCb()));

  // QSpacerItem* spacer =
  //     new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  // layout->addItem(spacer, index_, 5);

  UpdateConfiguration();
}

bool dispatcher::DispatcherItem::is_checked()
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
dispatcher::DispatcherItem::~DispatcherItem() {}

void dispatcher::DispatcherItem::SetEnabled(bool enable)
{
  enabled_ = enable;
  check_box_->setEnabled(enable);
  terminal_->setEnabled(enable);
  start_->setEnabled(enable);
  stop_->setEnabled(enable);
  if (enable) {
    label_->setPixmap(red_status_icon_);
  } else {
    label_->setPixmap(grey_status_icon_);
  }
}

void dispatcher::DispatcherItem::UpdateConfiguration()
{
  SetEnabled(true);

  if (tmux_initialized_) {
    TmuxKillSession();
  }

  std::string current_configuration = dispatcher_->get_current_configuration();
  
  if (configurations_.find(current_configuration) == configurations_.end()) {
    if (configurations_.find("all") == configurations_.end()) {
      // if default 'all' configuration also not found, then disable node
      SetEnabled(false);
      EVR_ACTIVITY_LO_REF(
          ros_node_,
          "Neither 'all' nor '%s' configuration found for node '%s'; disabling",
          current_configuration.c_str(), name_.c_str());
      return;
    } else {
      // selected configuration not found, use default configuration
      current_configuration_ = &configurations_["all"];
      EVR_ACTIVITY_LO_REF(ros_node_,
                          "No configurations specified for node '%s', "
                          "using built-in configuration 'all'",
                          name_.c_str());
    }
  } else {
    current_configuration_ = &configurations_[current_configuration];
    EVR_ACTIVITY_LO_REF(ros_node_, "Setting node '%s' to configuration '%s'",
                        name_.c_str(), current_configuration.c_str());
  }

  std::string cmd_tmp;
  const int   ssh_timeout_sec = ros_node_->get_ssh_timeout_sec();
  
  if (current_configuration_->hostname != "localhost") {
    if (current_configuration_->user.empty()) {
      cmd_tmp = "ssh -o PasswordAuthentication=no -o ConnectTimeout=" +
                std::to_string(ssh_timeout_sec) + " " +
                current_configuration_->hostname + " \"" +
                current_configuration_->cmd + "\"";
    } else {
      cmd_tmp = "ssh -o PasswordAuthentication=no -o ConnectTimeout=" +
                std::to_string(ssh_timeout_sec) + " " +
                current_configuration_->user + "@" +
                current_configuration_->hostname + " \"" +
                current_configuration_->cmd + "\"";
    }
  } else {
    cmd_tmp = current_configuration_->cmd;
  }
  start_->setToolTip(cmd_tmp.c_str());
}

void dispatcher::DispatcherItem::Process()
{
  if (!current_configuration_) {
    return;
  }

  if (!enabled_) {
    return;
  }

  const auto& online_nodes_             = ros_node_->get_online_nodes();
  size_t      num_online_nodes_found    = 0;
  size_t      num_expected_online_nodes = 0;

  // does current configuration have any ros nodes specified?
  // if yes, then use current configuration:
  if (current_configuration_->ros_nodes.size() > 0) {
    num_expected_online_nodes = current_configuration_->ros_nodes.size();
    // for each ros node in the current configuration:
    for (auto& ros_node : current_configuration_->ros_nodes) {
      // check if node exists in list of online nodes
      for (auto& online_node : online_nodes_) {
        if (ros_node.name == online_node.first &&
            ros_node.namespace_ == online_node.second) {
          num_online_nodes_found++;
          break;
        }
      }
    }
  } else {
    // for each ros node in the default configuration:
    num_expected_online_nodes = ros_nodes_.size();
    for (auto& ros_node : ros_nodes_) {
      // check if node exists in list of online nodes
      for (auto& online_node : online_nodes_) {
        if (ros_node.name == online_node.first &&
            ros_node.namespace_ == online_node.second) {
          num_online_nodes_found++;
          break;
        }
      }
    }
  }

  online_ = (num_online_nodes_found > 0);
  if (num_online_nodes_found != num_online_nodes_prev_) {
    EVR_ACTIVITY_LO_REF(
        ros_node_, "Status change for node %s, %ld/%ld nodes online",
        name_.c_str(), num_online_nodes_found, num_expected_online_nodes);
    num_online_nodes_prev_ = num_online_nodes_found;
    if (num_online_nodes_found == 0) {
      label_->setPixmap(red_status_icon_);
    } else if (num_online_nodes_found < num_expected_online_nodes) {
      label_->setPixmap(orange_status_icon_);
    } else if (num_online_nodes_found == num_expected_online_nodes) {
      label_->setPixmap(green_status_icon_);
    }
  }
}

void dispatcher::DispatcherItem::StartCb()
{
  if(!enabled_) {
    EVR_WARNING_LO_REF(
        ros_node_,
        "Refusing to start node %s in tmux session: %s because node is disabled",
        name_.c_str(), tmux_name_.c_str());
    return;
  }

  if (online_) {
    EVR_WARNING_HI_REF(
        ros_node_,
        "Refusing to start node %s in tmux session: %s "
        "because node was detected as already running on ROS2 message bus",
        name_.c_str(), tmux_name_.c_str());
    return;
  }

  EVR_ACTIVITY_HI_REF(ros_node_, "Starting node: %s in tmux session: %s",
                      name_.c_str(), tmux_name_.c_str());

  assert(current_configuration_);
  EVR_DIAGNOSTIC_REF(ros_node_, "system cmd: %s",
                     current_configuration_->cmd.c_str());

  if (!TmuxHasSession()) {
    EVR_ACTIVITY_LO_REF(
        ros_node_, "Tmux Session was closed for %s, restarting session now",
        name_.c_str());
    (void)TmuxNewSession();
  }

  // cd to workspace directory and source ros environment
  TmuxSendKeys("C-U");  // Clears the line of any text
  TmuxSendKeys("cd " + ros_node_->get_workspace());
  TmuxSendKeys("source install/setup.bash");

  // run user-supplied command, you can call an executable directly to start a
  // node or launch script
  std::string cmd_prefix;
  if (use_cmd_prefix_) {
    cmd_prefix =
        ros_node_->get_cmd_prefix(current_configuration_->configuration_name);
  }

  std::string env_prefix;
  if (use_environment_variables_) {
    for (auto& it : ros_node_->get_environment_variables(
             current_configuration_->configuration_name)) {
      env_prefix += it.first + "=" + it.second + " ";
    }
  }

  TmuxSendKeys(cmd_prefix + " " + env_prefix + " " +
               current_configuration_->cmd);

  if (attach_on_start_) {
    // detach any existing clients and attach a new gnome terminal window
    std::string cmd = "tmux detach-client -s " + tmux_name_;
    (void)SystemCall(cmd);
    TerminalCb();
  }
}

void dispatcher::DispatcherItem::StopCb()
{
  if (num_online_nodes_prev_ > 0) {
    EVR_ACTIVITY_LO_REF(ros_node_, "Stopping node: %s in tmux session: %s",
                        name_.c_str(), tmux_name_.c_str());
    TmuxSendKeys(stop_tmux_cmd_);
  }
}

void dispatcher::DispatcherItem::TerminalCb()
{
  if (!check_gnome_terminal_exists()) {
    EVR_WARNING_HI_REF(
        ros_node_,
        "Cannot attach to tmux session because 'gnome-terminal' does not exist "
        "on this "
        "system; you can attach to the session by running `tmux a -t %s` "
        "from any terminal",
        tmux_name_.c_str());
    return;
  }

  if (!TmuxHasSession()) {
    EVR_DIAGNOSTIC_REF(ros_node_,
                       "No tmux session was found for %s, starting new session",
                       tmux_name_.c_str());
    (void)TmuxNewSession();
  }

  // Start a gnome session and attach a tmux session
  std::string cmd             = "tmux a -t " + tmux_name_;
  const int   ssh_timeout_sec = ros_node_->get_ssh_timeout_sec();
  if (current_configuration_->hostname != "localhost") {
    if (current_configuration_->user.empty()) {
      cmd = "ssh -o PasswordAuthentication=no -o ConnectTimeout=" +
            std::to_string(ssh_timeout_sec) + " -t " +
            current_configuration_->hostname + " \"" + cmd + "\"";
    } else {
      cmd = "ssh -o PasswordAuthentication=no -o ConnectTimeout-" +
            std::to_string(ssh_timeout_sec) + " -t " +
            current_configuration_->user + "@" +
            current_configuration_->hostname + " \"" + cmd + "\"";
    }
  }
  EVR_ACTIVITY_LO_REF(ros_node_, "sending command: gnome-terminal -t %s -- %s",
                      name_.c_str(), cmd.c_str());
  system(("gnome-terminal -t " + name_ + " -- " + cmd).c_str());
}

bool dispatcher::DispatcherItem::TmuxKillSession()
{
  std::string cmd    = "tmux kill-session -t " + tmux_name_;
  int         result = SystemCall(cmd);
  if (result == 1) {
    EVR_WARNING_HI_REF(ros_node_, "tmux session %s could not be killed",
                       tmux_name_.c_str());
    return false;
  }
  return true;
}

bool dispatcher::DispatcherItem::TmuxNewSession()
{
  std::string cmd    = "tmux new -d -s " + tmux_name_;
  int         result = SystemCall(cmd);
  if (result == 1) {
    EVR_WARNING_HI_REF(ros_node_, "tmux session %s could not be created",
                       tmux_name_.c_str());
    return false;
  }
  tmux_initialized_ = true;
  return true;
}

void dispatcher::DispatcherItem::TmuxSendKeys(std::string cmd_str)
{
  std::string cmd =
      "tmux send-keys -t " + tmux_name_ + " '" + cmd_str + "' Enter";
  (void)SystemCall(cmd);
}

int dispatcher::DispatcherItem::SystemCall(std::string cmd)
{
  EVR_DIAGNOSTIC_REF(ros_node_, "Issuing subprocess call `%s`", cmd.c_str());
  std::string cmd_tmp         = cmd;
  const int   ssh_timeout_sec = ros_node_->get_ssh_timeout_sec();
  if (current_configuration_ &&
      current_configuration_->hostname != "localhost") {
    if (current_configuration_->user.empty()) {
      cmd_tmp = "ssh -o PasswordAuthentication=no -o ConnectTimeout=" +
                std::to_string(ssh_timeout_sec) + " " +
                current_configuration_->hostname + " \"" + cmd + "\"";
    } else {
      cmd_tmp = "ssh -o PasswordAuthentication=no -o ConnectTimeout=" +
                std::to_string(ssh_timeout_sec) + " " +
                current_configuration_->user + "@" +
                current_configuration_->hostname + " \"" + cmd + "\"";
    }
  }
  EVR_ACTIVITY_LO_REF(ros_node_, "SystemCall: %s", cmd_tmp.c_str());
  int result = system(cmd_tmp.c_str());
  EVR_DIAGNOSTIC_REF(ros_node_, "Subprocess call `%s` returned result %d",
                     cmd_tmp.c_str(), result);
  return result;
}

bool dispatcher::DispatcherItem::TmuxHasLocalSession()
{
  if (current_configuration_ &&
      current_configuration_->hostname == "localhost") {
    return TmuxHasSession();
  }
  return false;
}

bool dispatcher::DispatcherItem::TmuxHasSession()
{
  std::string cmd    = "tmux has-session -t " + tmux_name_ + " 2>/dev/null";
  int         result = SystemCall(cmd);
  if (result != 0) {
    EVR_DIAGNOSTIC_REF(ros_node_, "Tmux does not have active session for %s",
                       tmux_name_.c_str());
    return false;
  }
  EVR_DIAGNOSTIC_REF(ros_node_, "Tmux has active session for %s",
                     tmux_name_.c_str());
  return true;
}
