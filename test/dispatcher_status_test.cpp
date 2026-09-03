#include "dispatcher/detail/logic.h"

#include <gtest/gtest.h>

namespace dispatcher::detail
{
namespace
{

TEST(DispatcherStatusTest, SummarizeShellStatusReportsOfflineAndOnlineStates)
{
  const auto offline = SummarizeShellStatus(0);
  EXPECT_EQ(offline.found, 0u);
  EXPECT_EQ(offline.expected, 1u);
  EXPECT_FALSE(offline.online);
  EXPECT_EQ(offline.tooltip, "0/1 nodes online");
  EXPECT_EQ(offline.color, StatusColor::kRed);

  const auto online = SummarizeShellStatus(1);
  EXPECT_EQ(online.found, 1u);
  EXPECT_EQ(online.expected, 1u);
  EXPECT_TRUE(online.online);
  EXPECT_EQ(online.tooltip, "1/1 nodes online");
  EXPECT_EQ(online.color, StatusColor::kGreen);
}

TEST(DispatcherStatusTest, MakeFullyQualifiedNodeNameResolvesEveryNamespaceForm)
{
  // An omitted or bare-root namespace resolves to the root namespace, so a
  // monitored node spelled without one still matches the graph's '/talker'.
  EXPECT_EQ(MakeFullyQualifiedNodeName("", "talker"), "/talker");
  EXPECT_EQ(MakeFullyQualifiedNodeName("/", "talker"), "/talker");

  EXPECT_EQ(MakeFullyQualifiedNodeName("/demo", "talker"), "/demo/talker");
  EXPECT_EQ(MakeFullyQualifiedNodeName("/demo/", "talker"), "/demo/talker");
  EXPECT_EQ(MakeFullyQualifiedNodeName("demo", "talker"), "/demo/talker");

  // A name that is already absolute carries its own namespace.
  EXPECT_EQ(MakeFullyQualifiedNodeName("", "/fcat/fcat"), "/fcat/fcat");
  EXPECT_EQ(MakeFullyQualifiedNodeName("/", "/fcat/fcat"), "/fcat/fcat");
}

TEST(DispatcherStatusTest, SummarizeRosStatusMatchesRootNodesWithoutNamespace)
{
  // The ROS graph reports root-namespace nodes as ("talker", "/"). Monitoring
  // them with no namespace at all must still match.
  const std::vector<RosNodeMonitorConfig> configured_nodes = {
      {"", "talker"},
      {"", "listener"},
  };
  const std::vector<std::pair<std::string, std::string>> online_nodes = {
      {"talker", "/"},
      {"listener", "/"},
  };

  const auto status = SummarizeRosStatus({}, configured_nodes, online_nodes);

  EXPECT_EQ(status.found, 2u);
  EXPECT_EQ(status.expected, 2u);
  EXPECT_TRUE(status.online);
  EXPECT_EQ(status.color, StatusColor::kGreen);
  EXPECT_EQ(status.tooltip, "2/2 nodes online\n  /talker\n  /listener");
}

TEST(DispatcherStatusTest, SummarizeRosStatusMatchesFullyQualifiedNodeNames)
{
  // 'ros_nodes: - name: /fcat/fcat' style, with no namespace key.
  const std::vector<RosNodeMonitorConfig> configured_nodes = {
      {"", "/fcat/fcat"},
  };
  const std::vector<std::pair<std::string, std::string>> online_nodes = {
      {"fcat", "/fcat"},
  };

  const auto status = SummarizeRosStatus({}, configured_nodes, online_nodes);

  EXPECT_EQ(status.found, 1u);
  EXPECT_EQ(status.expected, 1u);
  EXPECT_EQ(status.color, StatusColor::kGreen);
}

TEST(DispatcherStatusTest, SummarizeRosStatusDoesNotMatchAcrossNamespaces)
{
  // '/talker' and '/demo/talker' are different nodes and must not be confused,
  // in either direction.
  const std::vector<RosNodeMonitorConfig> root_only = {{"/", "talker"}};
  const std::vector<RosNodeMonitorConfig> demo_only = {{"/demo", "talker"}};

  const std::vector<std::pair<std::string, std::string>> demo_online = {
      {"talker", "/demo"},
  };
  const std::vector<std::pair<std::string, std::string>> root_online = {
      {"talker", "/"},
  };

  EXPECT_EQ(SummarizeRosStatus({}, root_only, demo_online).found, 0u);
  EXPECT_EQ(SummarizeRosStatus({}, demo_only, root_online).found, 0u);
}

TEST(DispatcherStatusTest, SummarizeRosStatusUsesDefaultNodesWhenOverrideMissing)
{
  const std::vector<RosNodeMonitorConfig> default_nodes = {
      {"/", "alpha"},
      {"/robot", "beta"},
  };
  const std::vector<std::pair<std::string, std::string>> online_nodes = {
      {"alpha", "/"},
  };

  const auto status = SummarizeRosStatus(default_nodes, {}, online_nodes);

  EXPECT_EQ(status.found, 1u);
  EXPECT_EQ(status.expected, 2u);
  EXPECT_TRUE(status.online);
  EXPECT_EQ(status.color, StatusColor::kOrange);
  EXPECT_EQ(status.tooltip, "1/2 nodes online\n  /alpha");
}

TEST(DispatcherStatusTest, SummarizeRosStatusPrefersConfiguredNodeList)
{
  const std::vector<RosNodeMonitorConfig> default_nodes = {
      {"/", "alpha"},
  };
  const std::vector<RosNodeMonitorConfig> configured_nodes = {
      {"/science", "camera"},
      {"/robot", "beta"},
  };
  const std::vector<std::pair<std::string, std::string>> online_nodes = {
      {"camera", "/science"},
      {"beta", "/robot"},
      {"alpha", "/"},
  };

  const auto status =
      SummarizeRosStatus(default_nodes, configured_nodes, online_nodes);

  EXPECT_EQ(status.found, 2u);
  EXPECT_EQ(status.expected, 2u);
  EXPECT_TRUE(status.online);
  EXPECT_EQ(status.color, StatusColor::kGreen);
  EXPECT_EQ(status.tooltip,
            "2/2 nodes online\n  /science/camera\n  /robot/beta");
}

TEST(DispatcherStatusTest, SummarizeRosStatusStaysRedWhenNothingIsOnline)
{
  const std::vector<RosNodeMonitorConfig> configured_nodes = {
      {"/science", "camera"},
  };

  const auto status = SummarizeRosStatus({}, configured_nodes, {});

  EXPECT_EQ(status.found, 0u);
  EXPECT_EQ(status.expected, 1u);
  EXPECT_FALSE(status.online);
  EXPECT_EQ(status.color, StatusColor::kRed);
  EXPECT_EQ(status.tooltip, "0/1 nodes online");
}

}  // namespace
}  // namespace dispatcher::detail
