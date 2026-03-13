#include "dispatcher/process_item.h"
#include "dispatcher/dispatcher_widget.h"

#include <stdlib.h>

#include <algorithm>
#include <regex>
#include <string>

#include <QHBoxLayout>
#include <QIcon>
#include <QPixmap>
#include <QString>
#include <QVBoxLayout>

#include <rclcpp/rclcpp.hpp>

static bool check_gnome_terminal_exists()
{
  return (system("which gnome-terminal > /dev/null 2>&1") == 0) ? true : false;
}

/*!
@brief class constructor for ProcessItem
*/
dispatcher::ProcessItem::ProcessItem(QWidget*                    parent,
                                     dispatcher::DispatcherNode* ros_node,
                                     const YAML::Node&           node,
                                     QGridLayout*                layout)
    : dispatcher::Item(parent, ros_node, node)
{
  assert(layout);

  if (node["use_environment_variables"]) {
    use_environment_variables_ = node["use_environment_variables"].as<bool>();
  }

  if (node["attach_on_start"]) {
    attach_on_start_ = node["attach_on_start"].as<bool>();
  }

  if (node["cmd"] && node["configurations"]) {
    RCLCPP_FATAL(ros_node_->get_logger(),
                 "'cmd' and 'configurations' keys cannot be used together");
    rclcpp::shutdown();
  }

  if (node["configurations"]) {
    for (const auto& configuration : node["configurations"]) {
      dispatcher::Item::Config config;
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

      configurations_[config.configuration_name] = config;
    }
  }

  bool start_checked = node["start_checked"].as<bool>();
  index_             = layout->rowCount();

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

  int check_box_max_width    = 150;
  check_box_                 = new QCheckBox(QString(name_.c_str()), this);
  Qt::CheckState check_state = start_checked ? Qt::Checked : Qt::Unchecked;
  layout->addWidget(check_box_, index_, 0);
  check_box_->setChecked(check_state);
  check_box_->setAttribute(Qt::WA_TransparentForMouseEvents, false);
  check_box_->setFocusPolicy(Qt::StrongFocus);
  // Set min width for CheckBox so that resizing won't alter vertical alignment
  check_box_->setMinimumWidth(check_box_max_width);
  // If the Checkbox text is too long, convert to elidedText so we don't exceed
  // the min width for CheckBox
  QFontMetrics fontMetrics(check_box_->font());
  QString      elidedText = fontMetrics.elidedText(
      check_box_->text(), Qt::ElideMiddle, check_box_max_width);
  check_box_->setText(elidedText);

  QSpacerItem* spacer =
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  layout->addItem(spacer, index_, 1);

  label_ = new QLabel("");
  label_->setPixmap(red_status_icon_);
  layout->addWidget(label_, index_, 2);

  start_ = new QPushButton("start", this);
  start_->setStyleSheet(QString("color: green"));
  layout->addWidget(start_, index_, 3);
  connect(start_, SIGNAL(clicked()), this, SLOT(StartCb()));

  stop_ = new QPushButton("stop", this);
  stop_->setStyleSheet(QString("color: red"));
  layout->addWidget(stop_, index_, 4);
  connect(stop_, SIGNAL(clicked()), this, SLOT(StopCb()));

  terminal_ = new QPushButton(this);
  terminal_->setIcon(QIcon(":/icons/terminal.png"));
  terminal_->setIconSize(QSize(20, 20));
  layout->addWidget(terminal_, index_, 5);
  connect(terminal_, SIGNAL(clicked()), this, SLOT(TerminalCb()));

  UpdateConfiguration();
}

bool dispatcher::ProcessItem::is_checked()
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
dispatcher::ProcessItem::~ProcessItem() {}

void dispatcher::ProcessItem::SetEnabled(bool enable)
{
  enabled_ = enable;
  if (ros_node_->should_hide_unconfigured_process()) {
    check_box_->setVisible(enable);
    terminal_->setVisible(enable);
    start_->setVisible(enable);
    stop_->setVisible(enable);
    label_->setVisible(enable);
  } else {
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
}

void dispatcher::ProcessItem::UpdateConfiguration()
{
  SetEnabled(true);

  if (tmux_initialized_) {
    TmuxKillSession();
  }

  Item::UpdateConfiguration();
  if (current_configuration_ == nullptr) {
    // nullptr implies the current configuration is undefined for this item
    SetEnabled(false);
    return;
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

bool dispatcher::ProcessItem::PrepareTmuxSession()
{
  if (!enabled_) {
    RCLCPP_WARN(ros_node_->get_logger(),
                "Refusing to start node %s in tmux session: %s because "
                "node is disabled",
                name_.c_str(), tmux_name_.c_str());
    return false;
  }

  if (online_) {
    RCLCPP_WARN(ros_node_->get_logger(),
                "Refusing to start node %s in tmux session: %s "
                "because node was detected as already running",
                name_.c_str(), tmux_name_.c_str());
    return false;
  }

  RCLCPP_INFO(ros_node_->get_logger(), "Starting node: %s in tmux session: %s",
              name_.c_str(), tmux_name_.c_str());

  assert(current_configuration_);
  RCLCPP_DEBUG(ros_node_->get_logger(), "system cmd: %s",
               current_configuration_->cmd.c_str());

  if (!TmuxHasSession()) {
    RCLCPP_INFO(ros_node_->get_logger(),
                "Tmux Session was closed for %s, restarting session now",
                name_.c_str());
    (void)TmuxNewSession();
  }
  return true;
}

void dispatcher::ProcessItem::StartCb()
{
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

  // replace any instances of variables matching pattern $VARIABLE_NAME with its
  // value
  std::string configured_cmd = current_configuration_->cmd;
  const auto& variables      = ros_node_->GetVariables();
  for (auto& variable : variables) {
    configured_cmd = std::regex_replace(configured_cmd,
                                        std::regex("\\$" + variable->GetName()),
                                        variable->GetValue());
  }
  RCLCPP_INFO(ros_node_->get_logger(), "%s", configured_cmd.c_str());

  TmuxSendKeys(cmd_prefix + " " + env_prefix + " " + configured_cmd);

  if (attach_on_start_) {
    // detach any existing clients and attach a new gnome terminal window
    std::string cmd = "tmux detach-client -s " + tmux_name_;
    (void)SystemCall(cmd);
    TerminalCb();
  }
}

void dispatcher::ProcessItem::StopCb()
{
  if (num_online_nodes_prev_ > 0) {
    RCLCPP_INFO(ros_node_->get_logger(), "Stopping node: %s in tmux session: %s",
                name_.c_str(), tmux_name_.c_str());
    TmuxSendKeys(stop_tmux_cmd_);
  }
}

void dispatcher::ProcessItem::TerminalCb()
{
  if (!check_gnome_terminal_exists()) {
    RCLCPP_WARN(
        ros_node_->get_logger(),
        "Cannot attach to tmux session because 'gnome-terminal' does not exist "
        "on this "
        "system; you can attach to the session by running `tmux a -t %s` "
        "from any terminal",
        tmux_name_.c_str());
    return;
  }

  if (!TmuxHasSession()) {
    RCLCPP_DEBUG(ros_node_->get_logger(),
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
  RCLCPP_INFO(ros_node_->get_logger(),
              "sending command: gnome-terminal -t %s -- %s", name_.c_str(),
              cmd.c_str());
  int result = system(("gnome-terminal -t " + name_ + " -- " + cmd).c_str());
  (void)result;
}

bool dispatcher::ProcessItem::TmuxKillSession()
{
  std::string cmd    = "tmux kill-session -t " + tmux_name_;
  int         result = SystemCall(cmd);
  if (result == 1) {
    RCLCPP_WARN(ros_node_->get_logger(), "tmux session %s could not be killed",
                tmux_name_.c_str());
    return false;
  }
  return true;
}

bool dispatcher::ProcessItem::TmuxNewSession()
{
  std::string cmd    = "tmux new -d -s " + tmux_name_;
  int         result = SystemCall(cmd);
  if (result == 1) {
    RCLCPP_WARN(ros_node_->get_logger(),
                "tmux session %s could not be created", tmux_name_.c_str());
    return false;
  }
  tmux_initialized_ = true;
  return true;
}

void dispatcher::ProcessItem::TmuxSendKeys(std::string cmd_str)
{
  std::string cmd =
      "tmux send-keys -t " + tmux_name_ + " '" + cmd_str + "' Enter";
  (void)SystemCall(cmd);
}

bool dispatcher::ProcessItem::TmuxHasLocalSession()
{
  if (current_configuration_ &&
      current_configuration_->hostname == "localhost") {
    return TmuxHasSession();
  }
  return false;
}

bool dispatcher::ProcessItem::TmuxHasSession()
{
  std::string cmd    = "tmux has-session -t " + tmux_name_ + " 2>/dev/null";
  int         result = SystemCall(cmd);
  if (result != 0) {
    RCLCPP_DEBUG(ros_node_->get_logger(),
                 "Tmux does not have active session for %s",
                 tmux_name_.c_str());
    return false;
  }
  RCLCPP_DEBUG(ros_node_->get_logger(), "Tmux has active session for %s",
               tmux_name_.c_str());
  return true;
}
