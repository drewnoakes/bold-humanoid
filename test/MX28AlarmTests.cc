#include "gtest/gtest.h"

#include "../MX28Alarm/mx28alarm.hh"

using namespace bold;

TEST (MX28AlarmTests, basics)
{
  auto alarm = MX28Alarm(0);

  EXPECT_FALSE ( alarm.hasError() );

  EXPECT_FALSE ( alarm.hasInputVoltageError() );
  EXPECT_FALSE ( alarm.hasAngleLimitError() );
  EXPECT_FALSE ( alarm.hasOverheatedError() );
  EXPECT_FALSE ( alarm.hasRangeError() );
  EXPECT_FALSE ( alarm.hasChecksumError() );
  EXPECT_FALSE ( alarm.hasOverloadError() );
  EXPECT_FALSE ( alarm.hasInstructionError() );

  alarm = MX28Alarm(0x7F);

  EXPECT_TRUE ( alarm.hasError() );

  EXPECT_TRUE ( alarm.hasInputVoltageError() );
  EXPECT_TRUE ( alarm.hasAngleLimitError() );
  EXPECT_TRUE ( alarm.hasOverheatedError() );
  EXPECT_TRUE ( alarm.hasRangeError() );
  EXPECT_TRUE ( alarm.hasChecksumError() );
  EXPECT_TRUE ( alarm.hasOverloadError() );
  EXPECT_TRUE ( alarm.hasInstructionError() );
}

TEST (MX28AlarmTests, toString)
{
  EXPECT_EQ ( "", MX28Alarm(0).toString() );
  EXPECT_EQ ( "Input Voltage Limit Breached", MX28Alarm(1).toString() );
  EXPECT_EQ ( "Angle Limit Breached", MX28Alarm(2).toString() );
  EXPECT_EQ ( "Input Voltage Limit Breached, Angle Limit Breached", MX28Alarm(3).toString() );
}

TEST (MX28AlarmTests, assignFromUchar)
{
  MX28Alarm a;

  a = (uchar)2;
  EXPECT_EQ ( 2, a.getFlags() );

  a = (uchar)7;
  EXPECT_EQ ( 7, a.getFlags() );
}

TEST (MX28AlarmTests, set)
{
  MX28Alarm a(1 | 4);

  EXPECT_EQ(5, a.getFlags());
  EXPECT_FALSE(a.isSet(1));

  a.set(1, true);

  EXPECT_TRUE(a.isSet(1));
  EXPECT_EQ(7, a.getFlags());

  a.set(1, false);

  EXPECT_FALSE(a.isSet(1));
  EXPECT_EQ(5, a.getFlags());
}
