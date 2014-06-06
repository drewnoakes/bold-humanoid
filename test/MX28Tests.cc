#include "gtest/gtest.h"

#include "../MX28/mx28.hh"

using namespace bold;
using namespace std;

TEST (MX28Tests, getMirrorValue)
{
  vector<pair<ushort,ushort>> mirrors = {
    { 0x0800, 0x0800 },
    { 0x0001, 0x0FFF },
    { 0x0000, 0x0000 },
    { 0x0801, 0x07FF },
    { 2048, 2048 },
    { 2038, 2058 },
    { 3048, 1048 },
    { 4048, 48 }
  };

  for (auto const& pair : mirrors)
  {
    EXPECT_EQ( pair.first,  MX28::getMirrorValue(pair.second) ) << std::hex << pair.first << " should be the mirror of " << pair.second;
    EXPECT_EQ( pair.second, MX28::getMirrorValue(pair.first) ) << std::hex << pair.second << " should be the mirror of " << pair.first;
  }
}

TEST (MX28Tests, value2Rads)
{
  EXPECT_EQ(   0.0, MX28::value2Rads(0x0800) );
  EXPECT_NEAR(  M_PI, MX28::value2Rads(0x0FFF), 0.01 );
  EXPECT_NEAR( -M_PI, MX28::value2Rads(0x0000), 0.01 );
}

TEST (MX28Tests, value2Degs)
{
  EXPECT_EQ(    0.0, MX28::value2Degs(0x0800) );
  EXPECT_NEAR(  180.0, MX28::value2Degs(0x0FFF), 0.1 );
  EXPECT_NEAR( -180.0, MX28::value2Degs(0x0000), 0.1 );
}

TEST (MX28Tests, rads2Value)
{
  // TODO test outside range [-M_PI,M_PI]
  EXPECT_EQ( 0x0800, MX28::rads2Value(0.0) );
  // TODO why should this be off by one?
  EXPECT_EQ( 0x0FFF, MX28::rads2Value(M_PI) );
  EXPECT_EQ( 0x0000, MX28::rads2Value(-M_PI) );
}

TEST (MX28Tests, degs2Value)
{
  // TODO test outside range [-180,180]
  EXPECT_EQ( 0x0800, MX28::degs2Value(0.0) );
  // TODO why should this be off by one?
  EXPECT_EQ( 0x0FFF, MX28::degs2Value(180.0) );
  EXPECT_EQ( 0x0000, MX28::degs2Value(-180.0) );
}

TEST (MX28Tests, clampValue)
{
  EXPECT_EQ( 0, MX28::clampValue(-10) );
  EXPECT_EQ( 0, MX28::clampValue(0) );

  EXPECT_EQ( 0x0001, MX28::clampValue(0x0001) );
  EXPECT_EQ( 0x0800, MX28::clampValue(0x0800) );
  EXPECT_EQ( 0x0FFF, MX28::clampValue(0x0FFF) );
  EXPECT_EQ( 0x0FFF, MX28::clampValue(0x1000) );
}
