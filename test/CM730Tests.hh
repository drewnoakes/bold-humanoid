#include "gtest/gtest.h"

#include "helpers.hh"
#include "../CM730/cm730.hh"

TEST (CM730Tests, conversions)
{
  EXPECT_EQ( (uchar)0xFF, CM730::getHighByte(0xFF88) );
  EXPECT_EQ( (uchar)0x88, CM730::getLowByte(0xFF88) );
  EXPECT_EQ( 0xFF88, CM730::makeWord(0x88, 0xFF) );

  // Colours are encoded as BGR with 5 bits per channel
  // The 3 LSB of each channel are dropped

  EXPECT_EQ( 0x7C1F, CM730::color2Value(0xFF, 0x00, 0xFF) ); // 11111 00000 11111
  EXPECT_EQ( 0x5764, CM730::color2Value(0x20, 0xD8, 0xA8) ); // 10101 11011 00100
  EXPECT_EQ( 0x5764, CM730::color2Value(0x21, 0xD9, 0xA9) ); // same as prior, but with diff in LSB which is lost
}
