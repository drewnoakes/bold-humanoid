#include "gtest/gtest.h"

#include "helpers.hh"

#include "../ThreadId/threadid.hh"

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
      ThreadId::setThreadId(threadId);

      EXPECT_EQ( threadId, ThreadId::getThreadId() );
    };
  };

  int threadCount = 10;

  ThreadId::setThreadId((ThreadIds)0);

  vector<thread> threads;
  for (int t = 1; t <= threadCount; t++)
    threads.push_back(thread(makeLoop((ThreadIds)t)));

  for (auto& thread : threads)
    thread.join();

  EXPECT_EQ( (ThreadIds)0, ThreadId::getThreadId() );
}
