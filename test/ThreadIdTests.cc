#include "gtest/gtest.h"


#include "../ThreadUtil/threadutil.hh"

#include <thread>

using namespace bold;
using namespace std;

// NOTE these tests ensure the same threading characteristics on development and production environments

TEST (ThreadIdTests, threadIdAssignment)
{
  auto makeLoop = [](ThreadId threadId)
  {
    return [threadId]
    {
      ThreadUtil::setThreadId(threadId);

      EXPECT_EQ( threadId, ThreadUtil::getThreadId() );
    };
  };

  int threadCount = 10;

  ThreadUtil::setThreadId((ThreadId)0);

  vector<thread> threads;
  for (int t = 1; t <= threadCount; t++)
    threads.emplace_back(makeLoop((ThreadId)t));

  for (auto& thread : threads)
    thread.join();

  EXPECT_EQ( (ThreadId)0, ThreadUtil::getThreadId() );
}
