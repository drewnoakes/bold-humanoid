#include "gtest/gtest.h"

#include "helpers.hh"

#include "../ThreadUtil/threadutil.hh"

#include <thread>

using namespace bold;
using namespace std;

// NOTE these tests ensure the same threading characteristics on development and production environments

TEST (ThreadIdTests, threadIdAssignment)
{
  auto makeLoop = [](ThreadIds threadId)
  {
    return [threadId]()
    {
      ThreadUtil::setThreadId(threadId);

      EXPECT_EQ( threadId, ThreadUtil::getThreadId() );
    };
  };

  int threadCount = 10;

  ThreadUtil::setThreadId((ThreadIds)0);

  vector<thread> threads;
  for (int t = 1; t <= threadCount; t++)
    threads.push_back(thread(makeLoop((ThreadIds)t)));

  for (auto& thread : threads)
    thread.join();

  EXPECT_EQ( (ThreadIds)0, ThreadUtil::getThreadId() );
}
