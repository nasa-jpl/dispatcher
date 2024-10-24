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

/*!
@brief class constructor for ShellProcessItem
*/
dispatcher::ShellProcessItem::ShellProcessItem(
    QWidget* parent, dispatcher::DispatcherNode* ros_node,
    const YAML::Node& node)
    : dispatcher::ProcessItem(parent, ros_node, node)
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
  size_t      num_expected_online_nodes = 1;
  size_t      num_online_nodes_found    = num_online_nodes_prev_;
  std::string online_nodes_str;

  // Use pgrep to infer if process is running, but only once a second
  if (nticks_ % static_cast<int>(ros_node_->GetTimerRate()) == 0) {
    std::string cmd        = "pgrep " + name_ + " > /dev/null";
    num_online_nodes_found = (SystemCall(cmd, false) == 0) ? 1 : 0;
  }

  online_ = (num_online_nodes_found > 0);
  if (num_online_nodes_found != num_online_nodes_prev_) {
    online_nodes_str = std::to_string(num_online_nodes_found) + "/" +
                       std::to_string(num_expected_online_nodes) +
                       std::string(" nodes online");
    EVR_ACTIVITY_LO_REF(
        ros_node_, "Status change for node %s, %ld/%ld nodes online",
        name_.c_str(), num_online_nodes_found, num_expected_online_nodes);
    num_online_nodes_prev_ = num_online_nodes_found;
    label_->setToolTip(online_nodes_str.c_str());
    if (num_online_nodes_found == 0) {
      label_->setPixmap(red_status_icon_);
      label_->setStyleSheet(QString("color: red"));
    } else if (num_online_nodes_found < num_expected_online_nodes) {
      label_->setPixmap(orange_status_icon_);
      label_->setStyleSheet(QString("color: orange"));
    } else if (num_online_nodes_found == num_expected_online_nodes) {
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
