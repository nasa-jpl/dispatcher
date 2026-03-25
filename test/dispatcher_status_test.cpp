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
