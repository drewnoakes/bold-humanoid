#include "gtest/gtest.h"

#include <vector>

#include "helpers.hh"
#include "../util/consumerqueuethread.hh"

using namespace bold;
using namespace std;

TEST (ConsumerQueueThreadTests, basics)
{
  vector<int> values;

  auto testThreadId = this_thread::get_id();

  ConsumerQueueThread<int> queue([&values,&testThreadId](int i)
  {
    ASSERT_NE( testThreadId, this_thread::get_id() ) << "Should be on a different thread";
    values.push_back(i);
  });

  const int loopCount = 100;

  for (int i = 0; i < loopCount; i++)
  {
    queue.push(i);
    if (i % 20 == 0)
      this_thread::sleep_for(chrono::milliseconds(5));
  }

  queue.stop(); // joins

  ASSERT_TRUE( values.size() > 0 );
  ASSERT_TRUE( values.size() <= loopCount );

  for (int i = 0; i < values.size(); i++)
  {
    ASSERT_EQ ( i, values[i] );
  }
}

TEST (ConsumerQueueThreadTests, doesntBlockPusher)
{
  int callCount = 0;

  ConsumerQueueThread<int> queue([&](int i)
  {
    this_thread::sleep_for(chrono::milliseconds(1));
    callCount++;
  });

  int loopCount = 50;

  for (int i = 0; i < loopCount; i++)
    queue.push(1);

  ASSERT_TRUE ( callCount < loopCount );
  this_thread::sleep_for(chrono::milliseconds(loopCount / 2));
  ASSERT_TRUE ( callCount > 0 && callCount < loopCount ) << "Invalid callCount=" << callCount;
  this_thread::sleep_for(chrono::milliseconds(loopCount * 2));
  ASSERT_TRUE ( callCount == loopCount ) << "Invalid loopCount=" << loopCount;

  queue.stop(); // joins
}
