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

/*! @brief Visual state used by process-status indicators in the UI. */
enum class StatusColor {
  kRed = 0,
  kOrange,
  kGreen,
};

/*! @brief Aggregated monitoring result for a shell or ROS process item. */
struct MonitorStatus {
  size_t      found    = 0;
  size_t      expected = 0;
  bool        online   = false;
  std::string tooltip;
  StatusColor color = StatusColor::kRed;
};

/*!
@brief Resolves the effective command prefix for a configuration.
@param configurations Available dispatcher configurations keyed by name.
@param configuration Requested configuration name.
@return Pointer to the effective prefix string, or nullptr if no fallback exists.
*/
const std::string* ResolveCmdPrefix(
    const std::map<std::string, DispatcherNode::Configuration>& configurations,
    const std::string& configuration);

/*!
@brief Resolves the effective environment variables for a configuration.
@param configurations Available dispatcher configurations keyed by name.
@param configuration Requested configuration name.
@return Pointer to the effective environment-variable map, or nullptr if unavailable.
*/
const std::map<std::string, std::string>* ResolveEnvironmentVariables(
    const std::map<std::string, DispatcherNode::Configuration>& configurations,
    const std::string& configuration);

/*! @brief Parses a YAML item type string into a dispatcher enum value. */
DispatcherNode::ItemType ParseItemType(std::string type);
/*! @brief Converts a dispatcher enum value back to its YAML string form. */
std::string              ItemTypeToString(DispatcherNode::ItemType type);

/*!
@brief Builds the tmux session name used for a process item.
@param index Row index used as a stable numeric prefix.
@param name User-visible item name from YAML.
@return Tmux-safe session name with spaces replaced by underscores.
*/
std::string MakeTmuxName(int index, const std::string& name);

/*!
@brief Builds the launch tooltip shown on a process start button.
@return Local command text or the SSH-wrapped remote equivalent.
*/
std::string BuildLaunchToolTip(const std::string& cmd,
                               const std::string& hostname,
                               const std::string& user, int ssh_timeout_sec);

/*!
@brief Wraps a system command for remote execution when needed.
@return Local command text or the SSH command with dispatcher's control options.
*/
std::string WrapSystemCallCommand(const std::string& cmd,
                                  const std::string& hostname,
                                  const std::string& user,
                                  int                ssh_timeout_sec);

/*!
@brief Builds the tmux attach command used by the terminal button.
@return Local `tmux a -t ...` command or the SSH-wrapped remote attach command.
*/
std::string BuildTmuxAttachCommand(const std::string& tmux_name,
                                   const std::string& hostname,
                                   const std::string& user,
                                   int                ssh_timeout_sec);

/*!
@brief Replaces `$VARIABLE` tokens in a command string.
@param input Original command string.
@param variables Variable name/value pairs to substitute.
@return Command string with known variables replaced.
*/
std::string SubstituteVariables(
    const std::string&                                   input,
    const std::vector<std::pair<std::string, std::string>>& variables);

/*!
@brief Prepends command prefix and environment variables to a launch command.
@return Space-joined command ready to send to tmux.
*/
std::string BuildPrefixedCommand(
    const std::string&                      cmd_prefix,
    const std::map<std::string, std::string>& environment_variables,
    const std::string&                      configured_cmd);

/*!
@brief Builds the final shell command for a script button invocation.
@return `bash -c` command, optionally wrapped in `gnome-terminal --`.
*/
std::string BuildScriptSystemCall(
    const std::string&                      configured_cmd,
    const std::string&                      cmd_prefix,
    const std::map<std::string, std::string>& environment_variables,
    bool use_cmd_prefix, bool use_terminal);

/*!
@brief Summarizes shell-process liveness for UI display.
@param found Number of detected running processes.
@return Aggregated status, tooltip text, and indicator color.
*/
MonitorStatus SummarizeShellStatus(size_t found);

/*!
@brief Summarizes ROS node availability for UI display.
@param default_nodes Nodes monitored when no configuration-specific override exists.
@param configured_nodes Configuration-specific node overrides.
@param online_nodes Nodes currently reported by the ROS graph.
@return Aggregated status, tooltip text, and indicator color.
*/
MonitorStatus SummarizeRosStatus(
    const std::vector<RosNodeMonitorConfig>& default_nodes,
    const std::vector<RosNodeMonitorConfig>& configured_nodes,
    const std::vector<std::pair<std::string, std::string>>& online_nodes);

}  // namespace dispatcher::detail

#endif
