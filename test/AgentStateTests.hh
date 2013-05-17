#include "gtest/gtest.h"

#include "helpers.hh"

#include "../AgentState/agentstate.hh"
#include "../StateObject/TimingState/timingstate.hh"

#include <thread>
#include <vector>

using namespace bold;
using namespace std;

// NOTE these tests ensure the same threading characteristics on development and production environments

TEST (AgentStateTests, threadedAccess)
{
  int loopCount = 10000;
  
  AgentState::getInstance().registerStateType<MotionTimingState>("MotionTiming");
  
  thread producer([&]()
  {
    for (int i = 0; i < loopCount; i++)
    {
      auto eventTimings = make_shared<vector<EventTiming>>();
      eventTimings->push_back(make_pair(0.1, "Event 1"));
      eventTimings->push_back(make_pair(0.2, "Event 2"));
      AgentState::getInstance().set(make_shared<MotionTimingState const>(eventTimings));
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
