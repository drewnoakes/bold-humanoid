#include "gtest/gtest.h"

#include "helpers.hh"

#include <thread>
#include <vector>
#include <mutex>

using namespace std;

// NOTE these tests ensure the same threading characteristics on development and production environments

struct UnsafeCounter
{
  UnsafeCounter() : value(0) {}
  int value;
  void increment() { ++value; }
};

struct SafeCounter
{
  SafeCounter() : value(0) {}
  int value;
  void increment()
  {
    lock_guard<mutex> guard(d_mutex);
    ++value;
  }
private:
  mutex d_mutex;
};

TEST (ThreadTests, multipleCounters)
{
  int threadCount = 5;
  int iterationCount = 10000;
  int expected = threadCount * iterationCount;
  
  UnsafeCounter unsafeCounter;
  SafeCounter safeCounter;
  
  auto countLoop = [&]()
  {
    for (int i = 0; i < iterationCount; i++)
    {
      unsafeCounter.increment();
      safeCounter.increment();
    }
  };
  
  vector<thread> threads;
  for (int t = 0; t < threadCount; t++)
    threads.push_back(thread(countLoop));
  
  for (auto& thread : threads)
    thread.join();
  
  EXPECT_EQ( safeCounter.value,   expected );
  EXPECT_NE( unsafeCounter.value, expected );
}