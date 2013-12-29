#include "gtest/gtest.h"

#include "../util/schmitttrigger.hh"

using namespace bold;

TEST (SchmittTriggerTests, basics)
{
  SchmittTrigger<int> trigger(0, 10, true);

  EXPECT_EQ ( 0, trigger.getLowThreshold() );
  EXPECT_EQ ( 10, trigger.getHighThreshold() );

  EXPECT_TRUE ( trigger.isHigh() );

  EXPECT_EQ ( SchmittTriggerTransition::None, trigger.next(5) );

  EXPECT_TRUE ( trigger.isHigh() );

  EXPECT_EQ ( SchmittTriggerTransition::None, trigger.next(1) );
  EXPECT_EQ ( SchmittTriggerTransition::None, trigger.next(9) );
  EXPECT_EQ ( SchmittTriggerTransition::None, trigger.next(10) );
  EXPECT_EQ ( SchmittTriggerTransition::None, trigger.next(5) );
  EXPECT_EQ ( SchmittTriggerTransition::None, trigger.next(15) );
  EXPECT_EQ ( SchmittTriggerTransition::None, trigger.next(10) );
  EXPECT_EQ ( SchmittTriggerTransition::None, trigger.next(5) );
  EXPECT_EQ ( SchmittTriggerTransition::None, trigger.next(20) );
  EXPECT_TRUE ( trigger.isHigh() );
  EXPECT_EQ ( SchmittTriggerTransition::Low, trigger.next(-2) );
  EXPECT_FALSE ( trigger.isHigh() );
  EXPECT_EQ ( SchmittTriggerTransition::None, trigger.next(-2) );
  EXPECT_EQ ( SchmittTriggerTransition::None, trigger.next(5) );
  EXPECT_EQ ( SchmittTriggerTransition::None, trigger.next(-2) );
  EXPECT_EQ ( SchmittTriggerTransition::High, trigger.next(10) );
  EXPECT_TRUE ( trigger.isHigh() );
  EXPECT_EQ ( SchmittTriggerTransition::Low, trigger.next(0) );
  EXPECT_EQ ( SchmittTriggerTransition::High, trigger.next(11) );
  EXPECT_EQ ( SchmittTriggerTransition::Low, trigger.next(-1) );
}
