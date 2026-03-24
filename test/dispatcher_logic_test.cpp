#include "dispatcher/detail/logic.h"

#include <gtest/gtest.h>

namespace dispatcher::detail
{
namespace
{

TEST(DispatcherLogicTest, ParseItemTypeIsCaseInsensitive)
{
  EXPECT_EQ(ParseItemType("ROS"), dispatcher::DispatcherNode::ROS);
  EXPECT_EQ(ParseItemType("shell"), dispatcher::DispatcherNode::SHELL);
  EXPECT_EQ(ParseItemType("Category"), dispatcher::DispatcherNode::CATEGORY);
  EXPECT_EQ(ParseItemType("unknown"), dispatcher::DispatcherNode::UNDEF);
}

TEST(DispatcherLogicTest, ItemTypeToStringCoversAllEnums)
{
  EXPECT_EQ(ItemTypeToString(dispatcher::DispatcherNode::ROS), "ros");
  EXPECT_EQ(ItemTypeToString(dispatcher::DispatcherNode::SHELL), "shell");
  EXPECT_EQ(ItemTypeToString(dispatcher::DispatcherNode::CATEGORY), "category");
  EXPECT_EQ(ItemTypeToString(dispatcher::DispatcherNode::UNDEF), "undef");
}

TEST(DispatcherLogicTest, ParseRosNodeMonitorConfigDefaultsNamespaceToEmpty)
{
  const auto config = ParseRosNodeMonitorConfig(
      YAML::Load("{node_name: commander}"), "node_name");

  EXPECT_EQ(config.name, "commander");
  EXPECT_EQ(config.namespace_, "");
}

TEST(DispatcherLogicTest, ParseRosNodeMonitorConfigKeepsEmptyNamespace)
{
  const auto config = ParseRosNodeMonitorConfig(
      YAML::Load("{name: camera, namespace: \"\"}"), "name");

  EXPECT_EQ(config.name, "camera");
  EXPECT_EQ(config.namespace_, "");
}

TEST(DispatcherLogicTest, ResolveCmdPrefixFallsBackToAllWhenRequestedMissing)
{
  std::map<std::string, dispatcher::DispatcherNode::Configuration>
      configurations;
  configurations["all"].cmd_prefix = "GLOBAL=1";

  const auto* resolved = ResolveCmdPrefix(configurations, "offline");

  ASSERT_NE(resolved, nullptr);
  EXPECT_EQ(*resolved, "GLOBAL=1");
}

TEST(DispatcherLogicTest, ResolveCmdPrefixFallsBackToAllWhenRequestedEmpty)
{
  std::map<std::string, dispatcher::DispatcherNode::Configuration>
      configurations;
  configurations["all"].cmd_prefix     = "GLOBAL=1";
  configurations["online"].cmd_prefix  = "";
  configurations["remote"].cmd_prefix  = "REMOTE=1";

  const auto* online = ResolveCmdPrefix(configurations, "online");
  const auto* remote = ResolveCmdPrefix(configurations, "remote");

  ASSERT_NE(online, nullptr);
  ASSERT_NE(remote, nullptr);
  EXPECT_EQ(*online, "GLOBAL=1");
  EXPECT_EQ(*remote, "REMOTE=1");
}

TEST(DispatcherLogicTest, ResolveEnvironmentVariablesUsesRequestedWhenPresent)
{
  std::map<std::string, dispatcher::DispatcherNode::Configuration>
      configurations;
  configurations["all"].environment_variables["RMW"] = "fallback";
  configurations["online"].environment_variables["RMW"] = "cyclonedds";

  const auto* resolved = ResolveEnvironmentVariables(configurations, "online");

  ASSERT_NE(resolved, nullptr);
  ASSERT_EQ(resolved->size(), 1u);
  EXPECT_EQ(resolved->at("RMW"), "cyclonedds");
}

TEST(DispatcherLogicTest, MakeTmuxNameReplacesSpacesWithUnderscores)
{
  EXPECT_EQ(MakeTmuxName(3, "ping one two"), "3_ping_one_two");
}

TEST(DispatcherLogicTest, BuildLaunchToolTipBuildsRemoteCommandWithOptionalUser)
{
  EXPECT_EQ(BuildLaunchToolTip("run_me", "localhost", "", 10), "run_me");
  EXPECT_EQ(BuildLaunchToolTip("run_me", "robot", "", 10),
            "ssh -o PasswordAuthentication=no -o ConnectTimeout=10 robot \"run_me\"");
  EXPECT_EQ(BuildLaunchToolTip("run_me", "robot", "astro", 15),
            "ssh -o PasswordAuthentication=no -o ConnectTimeout=15 astro@robot \"run_me\"");
}

TEST(DispatcherLogicTest, WrapSystemCallCommandAddsPersistentSshOptionsForRemote)
{
  EXPECT_EQ(
      WrapSystemCallCommand("tmux has-session -t 1_ping", "robot", "astro", 9),
      "ssh -o PasswordAuthentication=no -o ControlPath=~/.ssh/cm-%r@%h:%p "
      "-o ControlMaster=auto -o ControlPersist=3m -o ConnectTimeout=9 "
      "astro@robot \"tmux has-session -t 1_ping\"");
}

TEST(DispatcherLogicTest, BuildTmuxAttachCommandUsesCorrectTimeoutSyntax)
{
  EXPECT_EQ(BuildTmuxAttachCommand("2_ping", "localhost", "", 5),
            "tmux a -t 2_ping");
  EXPECT_EQ(BuildTmuxAttachCommand("2_ping", "robot", "astro", 5),
            "ssh -o PasswordAuthentication=no -o ConnectTimeout=5 -t "
            "astro@robot \"tmux a -t 2_ping\"");
}

TEST(DispatcherLogicTest, SubstituteVariablesReplacesKnownTokensOnly)
{
  const std::vector<std::pair<std::string, std::string>> variables = {
      {"ROBOT", "astrobud"},
      {"MODE", "offline"},
  };

  EXPECT_EQ(SubstituteVariables("run $ROBOT in $MODE", variables),
            "run astrobud in offline");
  EXPECT_EQ(SubstituteVariables("keep $UNKNOWN intact", variables),
            "keep $UNKNOWN intact");
}

TEST(DispatcherLogicTest, BuildPrefixedCommandJoinsPrefixEnvAndCommand)
{
  const std::map<std::string, std::string> env_vars = {
      {"ALPHA", "1"},
      {"BETA", "2"},
  };

  EXPECT_EQ(BuildPrefixedCommand("PREFIX=1", env_vars, "ros2 run demo"),
            "PREFIX=1 ALPHA=1 BETA=2 ros2 run demo");
}

TEST(DispatcherLogicTest, BuildScriptSystemCallHonorsTerminalAndPrefixFlags)
{
  const std::map<std::string, std::string> env_vars = {
      {"ALPHA", "1"},
  };

  EXPECT_EQ(BuildScriptSystemCall("htop", "PREFIX=1", env_vars, true, false),
            "bash -c 'PREFIX=1 ALPHA=1 htop'");
  EXPECT_EQ(BuildScriptSystemCall("htop", "PREFIX=1", env_vars, true, true),
            "gnome-terminal -- bash -c 'PREFIX=1 ALPHA=1 htop'");
  EXPECT_EQ(BuildScriptSystemCall("htop", "PREFIX=1", env_vars, false, false),
            "bash -c 'htop'");
}

}  // namespace
}  // namespace dispatcher::detail
