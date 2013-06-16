#include "gtest/gtest.h"

#include "helpers.hh"

#include "../ThreadId/threadid.hh"

#include <thread>

using namespace bold;
using namespace std;

// NOTE these tests ensure the same threading characteristics on development and production environments

TEST (ThreadIdTests, threadIdAssignment)
{
  auto makeLoop = [](int threadId)
  {
    return [threadId]()
    {
      ThreadId::setThreadId(threadId);
      
      EXPECT_EQ( threadId, ThreadId::getThreadId() );
    };
  };
  
  int threadCount = 10;
  
  ThreadId::setThreadId(0);
  
  vector<thread> threads;
  for (int t = 1; t <= threadCount; t++)
    threads.push_back(thread(makeLoop(t)));
  
  for (auto& thread : threads)
    thread.join();
  
  EXPECT_EQ( 0, ThreadId::getThreadId() );
}
