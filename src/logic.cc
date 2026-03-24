#include "dispatcher/detail/logic.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <regex>

namespace dispatcher::detail
{
namespace
{

const dispatcher::DispatcherNode::Configuration* FindConfiguration(
    const std::map<std::string, dispatcher::DispatcherNode::Configuration>&
        configurations,
    const std::string& configuration)
{
  auto requested_it = configurations.find(configuration);
  auto all_it       = configurations.find("all");

  if (requested_it == configurations.end()) {
    return all_it == configurations.end() ? nullptr : &all_it->second;
  }
  return &requested_it->second;
}

std::string JoinCommandParts(const std::vector<std::string>& parts)
{
  std::string result;
  for (const auto& part : parts) {
    if (part.empty()) {
      continue;
    }
    if (!result.empty()) {
      result += " ";
    }
    result += part;
  }
  return result;
}

std::string BuildSshTarget(const std::string& hostname, const std::string& user)
{
  if (user.empty()) {
    return hostname;
  }
  return user + "@" + hostname;
}

StatusColor DetermineStatusColor(size_t found, size_t expected)
{
  if (found == 0) {
    return StatusColor::kRed;
  }
  if (found < expected) {
    return StatusColor::kOrange;
  }
  return StatusColor::kGreen;
}

std::string FormatOnlineNode(
    const std::pair<std::string, std::string>& online_node)
{
  if (online_node.second == "/") {
    return "\n  /" + online_node.first;
  }
  return "\n  " + online_node.second + "/" + online_node.first;
}

}  // namespace

RosNodeMonitorConfig ParseRosNodeMonitorConfig(const YAML::Node&   node,
                                               const std::string& name_key)
{
  RosNodeMonitorConfig config;
  config.name = node[name_key].as<std::string>();
  if (node["namespace"]) {
    config.namespace_ = node["namespace"].as<std::string>();
  }
  else {
    config.namespace_ = "";
  }
  return config;
}

const std::string* ResolveCmdPrefix(
    const std::map<std::string, DispatcherNode::Configuration>& configurations,
    const std::string& configuration)
{
  const auto* resolved = FindConfiguration(configurations, configuration);
  if (resolved == nullptr) {
    return nullptr;
  }
  if (!resolved->cmd_prefix.empty()) {
    return &resolved->cmd_prefix;
  }

  auto all_it = configurations.find("all");
  if (all_it != configurations.end()) {
    return &all_it->second.cmd_prefix;
  }
  return &resolved->cmd_prefix;
}

const std::map<std::string, std::string>* ResolveEnvironmentVariables(
    const std::map<std::string, DispatcherNode::Configuration>& configurations,
    const std::string& configuration)
{
  const auto* resolved = FindConfiguration(configurations, configuration);
  if (resolved == nullptr) {
    return nullptr;
  }
  if (!resolved->environment_variables.empty()) {
    return &resolved->environment_variables;
  }

  auto all_it = configurations.find("all");
  if (all_it != configurations.end()) {
    return &all_it->second.environment_variables;
  }
  return &resolved->environment_variables;
}

DispatcherNode::ItemType ParseItemType(std::string type)
{
  std::transform(type.begin(), type.end(), type.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  if (type == "shell") {
    return DispatcherNode::SHELL;
  }
  if (type == "ros") {
    return DispatcherNode::ROS;
  }
  if (type == "category") {
    return DispatcherNode::CATEGORY;
  }
  return DispatcherNode::UNDEF;
}

std::string ItemTypeToString(DispatcherNode::ItemType type)
{
  switch (type) {
    case DispatcherNode::ROS:
      return "ros";
    case DispatcherNode::SHELL:
      return "shell";
    case DispatcherNode::CATEGORY:
      return "category";
    default:
      return "undef";
  }
}

std::string MakeTmuxName(int index, const std::string& name)
{
  std::string sanitized_name = name;
  std::replace(sanitized_name.begin(), sanitized_name.end(), ' ', '_');
  return std::to_string(index) + "_" + sanitized_name;
}

std::string BuildLaunchToolTip(const std::string& cmd,
                               const std::string& hostname,
                               const std::string& user, int ssh_timeout_sec)
{
  if (hostname == "localhost") {
    return cmd;
  }
  return "ssh -o PasswordAuthentication=no -o ConnectTimeout=" +
         std::to_string(ssh_timeout_sec) + " " +
         BuildSshTarget(hostname, user) + " \"" + cmd + "\"";
}

std::string WrapSystemCallCommand(const std::string& cmd,
                                  const std::string& hostname,
                                  const std::string& user,
                                  int                ssh_timeout_sec)
{
  if (hostname == "localhost") {
    return cmd;
  }
  return "ssh -o PasswordAuthentication=no -o ControlPath=~/.ssh/cm-%r@%h:%p "
         "-o ControlMaster=auto -o ControlPersist=3m -o ConnectTimeout=" +
         std::to_string(ssh_timeout_sec) + " " +
         BuildSshTarget(hostname, user) + " \"" + cmd + "\"";
}

std::string BuildTmuxAttachCommand(const std::string& tmux_name,
                                   const std::string& hostname,
                                   const std::string& user,
                                   int                ssh_timeout_sec)
{
  std::string cmd = "tmux a -t " + tmux_name;
  if (hostname == "localhost") {
    return cmd;
  }
  return "ssh -o PasswordAuthentication=no -o ConnectTimeout=" +
         std::to_string(ssh_timeout_sec) + " -t " +
         BuildSshTarget(hostname, user) + " \"" + cmd + "\"";
}

std::string SubstituteVariables(
    const std::string&                                   input,
    const std::vector<std::pair<std::string, std::string>>& variables)
{
  std::string output = input;
  for (const auto& variable : variables) {
    output = std::regex_replace(output,
                                std::regex("\\$" + variable.first),
                                variable.second);
  }
  return output;
}

std::string BuildPrefixedCommand(
    const std::string&                      cmd_prefix,
    const std::map<std::string, std::string>& environment_variables,
    const std::string&                      configured_cmd)
{
  std::vector<std::string> parts;
  if (!cmd_prefix.empty()) {
    parts.push_back(cmd_prefix);
  }
  for (const auto& variable : environment_variables) {
    parts.push_back(variable.first + "=" + variable.second);
  }
  parts.push_back(configured_cmd);
  return JoinCommandParts(parts);
}

std::string BuildScriptSystemCall(
    const std::string&                      configured_cmd,
    const std::string&                      cmd_prefix,
    const std::map<std::string, std::string>& environment_variables,
    bool use_cmd_prefix, bool use_terminal)
{
  std::string script_cmd = configured_cmd;
  if (use_cmd_prefix) {
    script_cmd =
        BuildPrefixedCommand(cmd_prefix, environment_variables, configured_cmd);
  }

  std::string wrapped_cmd = "bash -c '" + script_cmd + "'";
  if (use_terminal) {
    return "gnome-terminal -- " + wrapped_cmd;
  }
  return wrapped_cmd;
}

MonitorStatus SummarizeShellStatus(size_t found)
{
  MonitorStatus status;
  status.found    = found;
  status.expected = 1;
  status.online   = (found > 0);
  status.tooltip =
      std::to_string(status.found) + "/" + std::to_string(status.expected) +
      " nodes online";
  status.color = DetermineStatusColor(status.found, status.expected);
  return status;
}

MonitorStatus SummarizeRosStatus(
    const std::vector<RosNodeMonitorConfig>& default_nodes,
    const std::vector<RosNodeMonitorConfig>& configured_nodes,
    const std::vector<std::pair<std::string, std::string>>& online_nodes)
{
  MonitorStatus status;
  const auto&   expected_nodes =
      configured_nodes.empty() ? default_nodes : configured_nodes;

  status.expected = expected_nodes.size();
  for (const auto& expected_node : expected_nodes) {
    // add / to name if not present
    std::string expected_name = expected_node.name;
    std::string expected_namespace = expected_node.namespace_;
    const auto expected_fullname = expected_namespace + expected_name;
    for (const auto& online_node : online_nodes) {
      std::string online_name = online_node.first;
      std::string online_namespace = online_node.second;
      // if namespace does not end with /, add it
      if (online_namespace.back() != '/') {
        online_namespace += "/";
      }
      const auto online_fullname = online_namespace + online_name;
      if (expected_fullname == online_fullname) {
        status.tooltip += FormatOnlineNode(online_node);
        status.found++;
        break;
      } 
    }
  }

  status.online = (status.found > 0);
  status.tooltip = std::to_string(status.found) + "/" +
                   std::to_string(status.expected) + " nodes online" +
                   status.tooltip;
  status.color = DetermineStatusColor(status.found, status.expected);
  return status;
}

}  // namespace dispatcher::detail
