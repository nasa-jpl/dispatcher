#ifndef DISPATCHER_DETAIL_LOGIC_H_
#define DISPATCHER_DETAIL_LOGIC_H_

#include "dispatcher/dispatcher_node.h"
#include "dispatcher/item.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace dispatcher::detail
{

enum class StatusColor {
  kRed = 0,
  kOrange,
  kGreen,
};

struct MonitorStatus {
  size_t      found    = 0;
  size_t      expected = 0;
  bool        online   = false;
  std::string tooltip;
  StatusColor color = StatusColor::kRed;
};

const std::string* ResolveCmdPrefix(
    const std::map<std::string, DispatcherNode::Configuration>& configurations,
    const std::string& configuration);

const std::map<std::string, std::string>* ResolveEnvironmentVariables(
    const std::map<std::string, DispatcherNode::Configuration>& configurations,
    const std::string& configuration);

DispatcherNode::ItemType ParseItemType(std::string type);
std::string              ItemTypeToString(DispatcherNode::ItemType type);

std::string MakeTmuxName(int index, const std::string& name);

std::string BuildLaunchToolTip(const std::string& cmd,
                               const std::string& hostname,
                               const std::string& user, int ssh_timeout_sec);

std::string WrapSystemCallCommand(const std::string& cmd,
                                  const std::string& hostname,
                                  const std::string& user,
                                  int                ssh_timeout_sec);

std::string BuildTmuxAttachCommand(const std::string& tmux_name,
                                   const std::string& hostname,
                                   const std::string& user,
                                   int                ssh_timeout_sec);

std::string SubstituteVariables(
    const std::string&                                   input,
    const std::vector<std::pair<std::string, std::string>>& variables);

std::string BuildPrefixedCommand(
    const std::string&                      cmd_prefix,
    const std::map<std::string, std::string>& environment_variables,
    const std::string&                      configured_cmd);

std::string BuildScriptSystemCall(
    const std::string&                      configured_cmd,
    const std::string&                      cmd_prefix,
    const std::map<std::string, std::string>& environment_variables,
    bool use_cmd_prefix, bool use_terminal);

MonitorStatus SummarizeShellStatus(size_t found);

MonitorStatus SummarizeRosStatus(
    const std::vector<RosNodeMonitorConfig>& default_nodes,
    const std::vector<RosNodeMonitorConfig>& configured_nodes,
    const std::vector<std::pair<std::string, std::string>>& online_nodes);

}  // namespace dispatcher::detail

#endif
