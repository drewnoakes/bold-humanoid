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
