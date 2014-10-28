#include <gtest/gtest.h>

#include "../DistributionTracker/distributiontracker.hh"

#include <cmath>

using namespace std;
using namespace bold;

TEST (DistributionTrackerTests, average)
{
  DistributionTracker tracker;

  tracker.add(1);
  tracker.add(2);
  tracker.add(3);

  EXPECT_EQ ( (1+2+3)/3.0, tracker.average() );
}

TEST (DistributionTrackerTests, stdDev)
{
  DistributionTracker tracker;

  tracker.add(10);
  tracker.add(12);
  tracker.add(13);

  ASSERT_NEAR ( 11.6666666667, tracker.average(), 0.0000001 );
  ASSERT_NEAR ( 1.55555555556, tracker.variance(), 0.0000001 );
  ASSERT_NEAR ( 1.24721912892, tracker.stdDev(), 0.0000001 );
}

TEST (DistributionTrackerTests, reset)
{
  DistributionTracker tracker;

  tracker.add(1);
  tracker.add(2);
  tracker.add(3);

  tracker.reset();

  tracker.add(2);
  tracker.add(2);
  tracker.add(2);

  ASSERT_EQ ( 2, tracker.average() );
  ASSERT_EQ ( 0, tracker.variance() );
  ASSERT_EQ ( 0, tracker.stdDev() );
}

TEST (DistributionTrackerTests, count)
{
  DistributionTracker tracker;

  for (int i = 0; i < 10; i++)
  {
    EXPECT_EQ   ( i, tracker.count() );
    tracker.add(i);
  }

  EXPECT_EQ   ( 10, tracker.count() );
}

TEST (DistributionTrackerTests, nanIfNothingAdded)
{
  DistributionTracker tracker;

  EXPECT_TRUE ( std::isnan(tracker.average()) );
  EXPECT_TRUE ( std::isnan(tracker.stdDev()) );
  EXPECT_EQ   ( 0, tracker.count() );
}

TEST (DistributionTrackerTests, throwsOnNaN)
{
  DistributionTracker tracker;

  EXPECT_THROW ( (tracker.add(NAN)), std::runtime_error );
}
