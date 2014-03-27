#include "gtest/gtest.h"

#include "helpers.hh"
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

TEST(DISABLED_SequentialTimerTests, enterExit)
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

TEST(DISABLED_SequentialTimerTests, nesting)
{
  SequentialTimer t;
  usleep(5000);
  t.timeEvent("a");

  t.enter("b");
  {
    t.enter("c");
    {
      t.enter("1");
      {
        usleep(7000);
      }
      t.exit();

      usleep(5000);
      t.timeEvent("2");
      usleep(2000);
      t.timeEvent("3");
    }
    t.exit();
  }
  t.exit();

  auto items = *t.flush();

  EXPECT_EQ(6, items.size());

  EXPECT_EQ("a",     items[0].second); EXPECT_BETWEEN( 5,  7, items[0].first);
  EXPECT_EQ("b/c/1", items[1].second); EXPECT_BETWEEN( 7,  9, items[1].first);
  EXPECT_EQ("b/c/2", items[2].second); EXPECT_BETWEEN( 5,  7, items[2].first);
  EXPECT_EQ("b/c/3", items[3].second); EXPECT_BETWEEN( 2,  4, items[3].first);
  EXPECT_EQ("b/c",   items[4].second); EXPECT_BETWEEN(14, 17, items[4].first);
  EXPECT_EQ("b",     items[5].second); EXPECT_BETWEEN(14, 17, items[5].first);

  EXPECT_NEAR(
    items[1].first + items[2].first + items[3].first,
    items[4].first, 1);

  EXPECT_NEAR(
    items[4].first,
    items[5].first, 0.1);
}

TEST(DISABLED_SequentialTimerTests, nesting2)
{
  SequentialTimer t;
  t.enter("a");
  {
    usleep(5000);
    t.enter("b");
    {
      usleep(5000);
      t.timeEvent("c");
      usleep(5000);
    }
    t.exit();
  }
  t.exit();

  auto items = *t.flush();

  EXPECT_EQ(3, items.size());

  EXPECT_EQ("a/b/c", items[0].second); EXPECT_BETWEEN( 5,  7, items[0].first);
  EXPECT_EQ("a/b",   items[1].second); EXPECT_BETWEEN(10, 12, items[1].first);
  EXPECT_EQ("a",     items[2].second); EXPECT_BETWEEN(15, 17, items[2].first);
}

TEST(SequentialTimerTests, nesting3)
{
  SequentialTimer t;
  usleep(50000);
  t.timeEvent("a");
  usleep(50000);
  t.enter("b"); // Should warn "Potential misuse of SequentialTimer: 50ms elapsed between last recording and enter"
  {
    usleep(50000);
    t.enter("c"); // And again here
    {
      usleep(50000);
    }
    t.exit();
    usleep(50000);
  }
  t.exit();

  auto items = *t.flush();

  ASSERT_EQ(3, items.size());

  EXPECT_EQ("a",   items[0].second); EXPECT_BETWEEN( 50,  70, items[0].first);
  EXPECT_EQ("b/c", items[1].second); EXPECT_BETWEEN( 50,  70, items[1].first);
  EXPECT_EQ("b",   items[2].second); EXPECT_BETWEEN(150, 190, items[2].first);
}
