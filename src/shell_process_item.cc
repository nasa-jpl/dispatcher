#include "dispatcher/detail/logic.h"
#include "dispatcher/shell_process_item.h"
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

/*!
@brief class constructor for ShellProcessItem
*/
dispatcher::ShellProcessItem::ShellProcessItem(
    QWidget* parent, dispatcher::DispatcherNode* ros_node,
    const YAML::Node& node, QGridLayout* layout)
    : dispatcher::ProcessItem(parent, ros_node, node, layout)
{
}

/*!
@brief class destructor
*/
dispatcher::ShellProcessItem::~ShellProcessItem() {}

void dispatcher::ShellProcessItem::Process()
{
  if (!current_configuration_) {
    return;
  }

  if (!enabled_) {
    return;
  }

  // In the case of a base DispatcherItem, there's only ever one "node"
  size_t      num_online_nodes_found    = num_online_nodes_prev_;

  // Use pgrep to infer if process is running, but only once a second
  if (nticks_ % static_cast<int>(ros_node_->GetTimerRate()) == 0) {
    std::string cmd        = "pgrep " + name_ + " > /dev/null";
    num_online_nodes_found = (SystemCall(cmd, false) == 0) ? 1 : 0;
  }

  const auto status = dispatcher::detail::SummarizeShellStatus(
      num_online_nodes_found);
  online_ = status.online;
  if (status.found != num_online_nodes_prev_) {
    RCLCPP_INFO(ros_node_->get_logger(),
                "Status change for node %s, %ld/%ld nodes online",
                name_.c_str(), status.found, status.expected);
    num_online_nodes_prev_ = status.found;
    label_->setToolTip(status.tooltip.c_str());
    if (status.color == dispatcher::detail::StatusColor::kRed) {
      label_->setPixmap(red_status_icon_);
      label_->setStyleSheet(QString("color: red"));
    } else if (status.color == dispatcher::detail::StatusColor::kOrange) {
      label_->setPixmap(orange_status_icon_);
      label_->setStyleSheet(QString("color: orange"));
    } else {
      label_->setPixmap(green_status_icon_);
      label_->setStyleSheet(QString("color: green"));
    }
  }
  nticks_++;
}

void dispatcher::ShellProcessItem::StartCb()
{
  if (!dispatcher::ProcessItem::PrepareTmuxSession()) {
    return;
  }

  dispatcher::ProcessItem::StartCb();
}

void dispatcher::ShellProcessItem::StopCb()
{
  dispatcher::ProcessItem::StopCb();
}

void dispatcher::ShellProcessItem::TerminalCb()
{
  dispatcher::ProcessItem::TerminalCb();
}
