#include "gtest/gtest.h"


#include "../State/state.hh"
#include "../StateObject/TimingState/timingstate.hh"

#include <atomic>
#include <thread>
#include <mutex>
#include <vector>

using namespace bold;
using namespace std;

// NOTE these tests ensure the same threading characteristics on development and production environments

class StateTests : public ::testing::Test
{
protected:
  static void SetUpTestCase()
  {
    State::registerStateType<MotionTimingState>("MotionTiming");
  }
};

TEST_F (StateTests, threadedAccess)
{
  int loopCount = 50000;

  thread producer([&]()
  {
    for (int i = 0; i < loopCount; i++)
    {
      auto eventTimings = make_shared<vector<EventTiming>>();
      eventTimings->push_back(make_pair(0.1, "Event 1"));
      eventTimings->push_back(make_pair(0.2, "Event 2"));
      State::make<MotionTimingState>(eventTimings, 1);
    }
  });

  bool seenState = false;

  thread consumer([&]()
  {
    for (int i = 0; i < loopCount; i++)
    {
      auto const& state = State::get<MotionTimingState>();

      if (state)
      {
        EXPECT_EQ(2, state->getTimings()->size());
        seenState = true;
      }
    }
  });

  producer.join();
  consumer.join();

  EXPECT_TRUE(seenState);
}

TEST_F (StateTests, setAndGet)
{
  auto eventTimings = make_shared<vector<EventTiming>>();
  eventTimings->push_back(make_pair(0.1, "Event 1"));
  eventTimings->push_back(make_pair(0.2, "Event 2"));
  State::make<MotionTimingState>(eventTimings, 2);

  shared_ptr<MotionTimingState const> state = State::get<MotionTimingState>();

  EXPECT_NE ( nullptr, state );
  EXPECT_EQ(2, state->getTimings()->size());
}
