#include <gtest/gtest.h>

#include "../util/Range.hh"

using namespace std;
using namespace bold;

TEST (RangeTests, construction)
{
  auto range = Range<int>();

  EXPECT_TRUE ( range.isEmpty() );

  range = Range<int>(1, 2);

  EXPECT_FALSE ( range.isEmpty() );
  EXPECT_EQ ( 1, range.min() );
  EXPECT_EQ ( 2, range.max() );
}

TEST (RangeTests, equality)
{
  EXPECT_EQ ( Range<int>(), Range<int>() );
  EXPECT_EQ ( Range<int>(1, 2), Range<int>(1, 2) );

  EXPECT_NE ( Range<int>(), Range<int>(1, 1) );
  EXPECT_NE ( Range<int>(1, 2), Range<int>() );
  EXPECT_NE ( Range<int>(1, 2), Range<int>(1, 1) );
  EXPECT_NE ( Range<int>(1, 2), Range<int>(2, 2) );

  // Equal after reset
  auto range = Range<int>(1, 2);
  range.reset();
  EXPECT_EQ ( Range<int>(), range );
}

TEST (RangeTests, expand)
{
  auto range = Range<int>(1, 2);

  range.expand(1); // NOOP

  EXPECT_EQ ( Range<int>(1, 2), range );

  range.expand(0);

  EXPECT_EQ ( Range<int>(0, 2), range );

  range.expand(3);

  EXPECT_EQ ( Range<int>(0, 3), range );
}

TEST (RangeTests, expandWithRange)
{
  auto range = Range<int>(1, 2);

  range.expand(Range<int>(0, 1));

  EXPECT_EQ ( Range<int>(0, 2), range );

  range.expand(Range<int>(5, 6));

  EXPECT_EQ ( Range<int>(0, 6), range );
}

TEST (RangeTests, reset)
{
  auto range = Range<int>(1, 2);

  range.reset();

  EXPECT_TRUE ( range.isEmpty() );

  range.expand(1);

  EXPECT_FALSE ( range.isEmpty() );
  EXPECT_EQ ( 1, range.min() );
  EXPECT_EQ ( 1, range.max() );
}

TEST (RangeTests, size)
{
  EXPECT_EQ ( 0, Range<int>().size() );
  EXPECT_EQ ( 0, Range<int>(1, 1).size() );
  EXPECT_EQ ( 1, Range<int>(1, 2).size() );
  EXPECT_EQ ( 2, Range<int>(1, 3).size() );
  EXPECT_EQ ( 1, Range<int>(2, 3).size() );
  EXPECT_EQ ( 4, Range<int>(-1, 3).size() );
}

TEST (RangeTests, contains)
{
  EXPECT_TRUE ( Range<int>(0, 10).contains(0) );
  EXPECT_TRUE ( Range<int>(0, 10).contains(5) );
  EXPECT_TRUE ( Range<int>(0, 10).contains(10) );

  EXPECT_FALSE( Range<int>(0, 10).contains(-1) );
  EXPECT_FALSE( Range<int>(0, 10).contains(11) );
}

TEST (RangeTests, clamp)
{
  EXPECT_EQ ( 5,  Range<int>(0, 10).clamp(5) );
  EXPECT_EQ ( 0,  Range<int>(0, 10).clamp(0) );
  EXPECT_EQ ( 10, Range<int>(0, 10).clamp(10) );
  EXPECT_EQ ( 0,  Range<int>(0, 10).clamp(-1) );
  EXPECT_EQ ( 10,  Range<int>(0, 10).clamp(11) );

  EXPECT_EQ ( 0,   Range<int>(-10, 10).clamp(0) );
  EXPECT_EQ ( -5,  Range<int>(-10, 10).clamp(-5) );
  EXPECT_EQ ( -10, Range<int>(-10, 10).clamp(-10) );
  EXPECT_EQ ( 10,  Range<int>(-10, 10).clamp(10) );
}

