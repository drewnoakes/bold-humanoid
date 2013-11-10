#include "gtest/gtest.h"

#include "helpers.hh"

#include "../AgentState/agentstate.hh"
#include "../StateObject/TimingState/timingstate.hh"

#include <atomic>
#include <thread>
#include <mutex>
#include <vector>

using namespace bold;
using namespace std;

// NOTE these tests ensure the same threading characteristics on development and production environments

class AgentStateTests : public ::testing::Test
{
protected:
  static void SetUpTestCase()
  {
    AgentState::getInstance().registerStateType<MotionTimingState>("MotionTiming");
  }
};

TEST_F (AgentStateTests, threadedAccess)
{
  int loopCount = 50000;

  thread producer([&]()
  {
    for (int i = 0; i < loopCount; i++)
    {
      auto eventTimings = make_shared<vector<EventTiming>>();
      eventTimings->push_back(make_pair(0.1, "Event 1"));
      eventTimings->push_back(make_pair(0.2, "Event 2"));
      AgentState::getInstance().set(make_shared<MotionTimingState const>(eventTimings, 1));
    }
  });

  bool seenState = false;

  thread consumer([&]()
  {
    for (int i = 0; i < loopCount; i++)
    {
      auto const& state = AgentState::get<MotionTimingState>();

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

TEST_F (AgentStateTests, setAndGet)
{
  auto eventTimings = make_shared<vector<EventTiming>>();
  eventTimings->push_back(make_pair(0.1, "Event 1"));
  eventTimings->push_back(make_pair(0.2, "Event 2"));
  AgentState::getInstance().set(make_shared<MotionTimingState const>(eventTimings, 2));

  shared_ptr<MotionTimingState const> state = AgentState::get<MotionTimingState>();

  EXPECT_NE ( nullptr, state );
  EXPECT_EQ(2, state->getTimings()->size());
}
