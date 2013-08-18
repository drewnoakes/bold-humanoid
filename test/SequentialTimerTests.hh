#include "gtest/gtest.h"

#include "../SequentialTimer/sequentialtimer.hh"

#include <memory>
#include <chrono>
#include <thread>

using namespace bold;
using namespace std;

TEST(SequentialTimerTests, basics)
{
  SequentialTimer t;
  usleep(2000); // sleep for 1ms
  t.timeEvent("2");
  t.timeEvent("0");
  auto items = *t.flush();

  EXPECT_EQ(2, items.size());

  EXPECT_EQ("2", items[0].second);
  EXPECT_EQ("0", items[1].second);

  EXPECT_NEAR(2, items[0].first, 0.75);
  EXPECT_NEAR(0, items[1].first, 0.75);

  EXPECT_TRUE(items[0].first > items[1].first);
}

TEST(SequentialTimerTests, enterExit)
{
  SequentialTimer t;

  EXPECT_EQ("", t.getPrefix());

  t.timeEvent("1");

  t.enter("a");
  EXPECT_EQ("a", t.getPrefix());

  t.timeEvent("2");

  t.enter("b");
  EXPECT_EQ("a/b", t.getPrefix());

  t.timeEvent("3");

  t.exit();
  EXPECT_EQ("a", t.getPrefix());

  t.timeEvent("4");

  t.exit();
  EXPECT_EQ("", t.getPrefix());

  t.timeEvent("5");

  auto items = *t.flush();

  EXPECT_EQ(7, items.size());

  EXPECT_EQ("1", items[0].second);
  EXPECT_EQ("a/2", items[1].second);
  EXPECT_EQ("a/b/3", items[2].second);
  EXPECT_EQ("a/b", items[3].second);
  EXPECT_EQ("a/4", items[4].second);
  EXPECT_EQ("a", items[5].second);
  EXPECT_EQ("5", items[6].second);
}
