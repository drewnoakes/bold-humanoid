#include "gtest/gtest.h"

#include "../util/Range.hh"

using namespace std;
using namespace bold;

TEST (RangeTests, cosntruction)
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
  
  EXPECT_FALSE ( range.isEmpty() );
  EXPECT_EQ ( 1, range.min() );
  EXPECT_EQ ( 2, range.max() );
  
  range.expand(1); // NOOP
  
  EXPECT_FALSE ( range.isEmpty() );
  EXPECT_EQ ( 1, range.min() );
  EXPECT_EQ ( 2, range.max() );
  
  EXPECT_FALSE ( range.isEmpty() );
  
  EXPECT_EQ ( Range<int>(1, 2), range );
  
  range.expand(0);
  
  EXPECT_EQ ( Range<int>(0, 2), range );
  
  range.expand(3);
  
  EXPECT_EQ ( Range<int>(0, 3), range );
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