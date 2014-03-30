#include "gtest/gtest.h"


#include <thread>
#include <vector>
#include <mutex>
#include <sigc++/sigc++.h>

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

TEST (ThreadTests, DISABLED_multipleCounters)
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
    threads.emplace_back(countLoop);

  for (auto& thread : threads)
    thread.join();

  EXPECT_EQ( safeCounter.value,   expected );
  EXPECT_NE( unsafeCounter.value, expected );
}

TEST (ThreadTests, DISABLED_threadSafetyOfSignals)
{
  int threadCount = 5;
  int iterationCount = 10000;
//   int expected = threadCount * iterationCount;

  int count;
  sigc::signal<void> sig;
  mutex m;

  sig.connect([&](){ count++; });

  vector<thread> threads;
  for (int t = 0; t < threadCount; t++)
  {
    threads.emplace_back([&]()
    {
      for (int i = 0; i < iterationCount; i++)
      {
        lock_guard<mutex> guard(m);
        sig();
      }
    });
  }

  for (auto& thread : threads)
    thread.join();

//   EXPECT_EQ( safeCounter.value,   expected );
//   EXPECT_NE( unsafeCounter.value, expected );
}

TEST (ThreadTests, DISABLED_threadedProducerConsumer)
{
  int loopCount = 50000;

  shared_ptr<int> ptr;
  mutex m;

  thread producer([loopCount,&ptr,&m]()
  {
    for (int i = 0; i < loopCount; i++)
    {
//       auto a = make_shared<int>();
//       atomic_store<int>(&ptr, a);
//       std::atomic_store<int>(&ptr, make_shared<B>());
      lock_guard<mutex> guard(m);
      ptr = make_shared<int>(i);
    }
  });

  thread consumer([loopCount,&ptr,&m]()
  {
    int lastVal = 0;
    for (int i = 0; i < loopCount; i++)
    {
//       atomic<int> const p = ptr;
//       auto state = std::atomic_load<int>(&p);
      lock_guard<mutex> guard(m);
      shared_ptr<int> state = ptr;
      EXPECT_TRUE( *state >= lastVal );
      lastVal = *state;
    }
  });

  producer.join();
  consumer.join();
}
