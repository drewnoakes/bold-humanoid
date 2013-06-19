#include "gtest/gtest.h"

#include "../util/conditionals.hh"

#include <memory>
#include <chrono>
#include <thread>

using namespace bold;
using namespace std;

TEST(ConditionalsTests, negateFunction)
{
  bool state = true;
  auto flop = [&state]()
  {
    state = !state;
    return state;
  };

  EXPECT_FALSE(flop());
  EXPECT_TRUE(flop());
  EXPECT_FALSE(flop());
  EXPECT_TRUE(flop());

  auto negativeFlop = bold::negate(flop);

  EXPECT_TRUE(negativeFlop());
  EXPECT_FALSE(negativeFlop());
  EXPECT_TRUE(negativeFlop());
  EXPECT_FALSE(negativeFlop());
}

TEST(ConditionalsTests, trueNTimes)
{
  auto trueOnce = trueNTimes(1);

  EXPECT_TRUE(trueOnce());
  EXPECT_FALSE(trueOnce());
  EXPECT_FALSE(trueOnce());

  auto trueTwice = trueNTimes(2);

  EXPECT_TRUE(trueTwice());
  EXPECT_TRUE(trueTwice());
  EXPECT_FALSE(trueTwice());
  EXPECT_FALSE(trueTwice());
}

TEST(ConditionalsTests, isRepeatedFunction)
{
  auto repeatedThrice = isRepeated(3, trueNTimes(4));

  EXPECT_FALSE(repeatedThrice());
  EXPECT_FALSE(repeatedThrice());
  EXPECT_TRUE(repeatedThrice());
  EXPECT_TRUE(repeatedThrice());
  EXPECT_FALSE(repeatedThrice());
  EXPECT_FALSE(repeatedThrice());
}

TEST(ConditionalsTests, trueForMillis)
{
  bool value = true;
  auto fun = trueForMillis(10, [&value](){ return value; });

  EXPECT_FALSE(fun());
  EXPECT_FALSE(fun());
  EXPECT_FALSE(fun());
  this_thread::sleep_for(chrono::milliseconds(15));
  EXPECT_TRUE(fun());
  EXPECT_TRUE(fun());
  value = false;
  EXPECT_FALSE(fun());

  // TODO should state reset after signaling true?
}

TEST(ConditionalsTests, stepUpDownThreshold)
{
  bool value = true;
  auto fun = stepUpDownThreshold(3, [&value](){ return value; });

  EXPECT_FALSE(fun()); // 1
  EXPECT_FALSE(fun()); // 2
  value = false;
  EXPECT_FALSE(fun()); // 1
  EXPECT_FALSE(fun()); // 0
  value = true;
  EXPECT_FALSE(fun()); // 1
  EXPECT_FALSE(fun()); // 2
  EXPECT_TRUE(fun());  // 3
  EXPECT_TRUE(fun());  // 3
  EXPECT_TRUE(fun());  // 3
  value = false;
  EXPECT_FALSE(fun()); // 2
  value = true;
  EXPECT_TRUE(fun());  // 3
}

TEST(ConditionalsTests, oneShot)
{
  int createCount = 0;
  bool value = false;
  
  auto fun = oneShot([&createCount,&value]()
  {
    createCount++;
    return [&value]() { return value; };
  });

  EXPECT_EQ(0, createCount) << "Nested function not called until needed";

  EXPECT_FALSE(fun());
  EXPECT_EQ(1, createCount);
  EXPECT_FALSE(fun());
  EXPECT_EQ(1, createCount);

  value = true;
  EXPECT_TRUE(fun());
  EXPECT_EQ(1, createCount) << "Nested function not recreated yet";

  EXPECT_TRUE(fun());
  EXPECT_EQ(2, createCount) << "Nested function not recreated yet";

  value = false;

  EXPECT_FALSE(fun());
  EXPECT_EQ(3, createCount);
  EXPECT_FALSE(fun());
  EXPECT_EQ(3, createCount);
}
