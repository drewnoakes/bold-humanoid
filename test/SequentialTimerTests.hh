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
  usleep(5000); // sleep (microseconds)
  t.timeEvent("2");
  t.timeEvent("0");
  auto items = *t.flush();

  EXPECT_EQ(2, items.size());

  EXPECT_EQ("2", items[0].second);
  EXPECT_EQ("0", items[1].second);

  EXPECT_NEAR(5, items[0].first, 2.0);
  EXPECT_NEAR(0, items[1].first, 2.0);

  EXPECT_TRUE(items[0].first > items[1].first);
}

TEST(SequentialTimerTests, enterExit)
{
  SequentialTimer t;

  EXPECT_EQ("", t.getPrefix());

  t.timeEvent("1");

  t.enter("a");
  {
    EXPECT_EQ("a", t.getPrefix());
    usleep(5000);

    t.timeEvent("2");

    t.enter("b");
    {
      EXPECT_EQ("a/b", t.getPrefix());
      usleep(5000);

      t.timeEvent("3");
    }
    t.exit();

    EXPECT_EQ("a", t.getPrefix());

    usleep(5000);
    t.timeEvent("4");
  }
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

  EXPECT_NEAR(
    items[1].first + items[3].first + items[4].first,
    items[5].first, 2); // within 2ms
}


TEST(SequentialTimerTests, nesting)
{
  SequentialTimer t;
  t.enter("a");
  {
    t.enter("b");
    {
      usleep(5000);
      t.timeEvent("1");
      usleep(2000);
      t.timeEvent("2");
    }
    t.exit();
  }
  t.exit();

  auto items = *t.flush();

  EXPECT_EQ(4, items.size());

  EXPECT_EQ("a/b/1", items[0].second);
  EXPECT_EQ("a/b/2", items[1].second);
  EXPECT_EQ("a/b",   items[2].second);
  EXPECT_EQ("a",     items[3].second);

  EXPECT_NEAR(
    items[0].first + items[1].first,
    items[3].first, 1);

  EXPECT_NEAR(
    items[0].first + items[1].first,
    items[3].first, 1);
}
