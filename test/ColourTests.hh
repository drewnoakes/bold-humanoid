#include "gtest/gtest.h"

#include "../Colour/colour.hh"

using namespace bold;

TEST (ColourTests, bgr2hsv)
{
  Colour::bgr red(0,0,255);
  Colour::bgr blue(255,0,0);
  Colour::bgr green(0,255,0);
  Colour::bgr yellow(0,255,255);
  Colour::bgr cyan(255,255,0);
  Colour::bgr magenta(255,0,255);
  Colour::bgr gray(128,128,128);
  Colour::bgr black(0,0,0);
  Colour::bgr white(255,255,255);

  EXPECT_EQ ( Colour::hsv(0, 255, 255), Colour::bgr2hsv(red) );
  EXPECT_EQ ( Colour::hsv(85, 255, 255), Colour::bgr2hsv(green) );
  EXPECT_EQ ( Colour::hsv(170, 255, 255), Colour::bgr2hsv(blue) );
  EXPECT_EQ ( Colour::hsv(42, 255, 255), Colour::bgr2hsv(yellow) );
  EXPECT_EQ ( Colour::hsv(127, 255, 255), Colour::bgr2hsv(cyan) );
  EXPECT_EQ ( Colour::hsv(212, 255, 255), Colour::bgr2hsv(magenta) );
  EXPECT_EQ ( Colour::hsv(0, 0, 128), Colour::bgr2hsv(gray) );
  EXPECT_EQ ( Colour::hsv(0, 0, 0), Colour::bgr2hsv(black) );
  EXPECT_EQ ( Colour::hsv(0, 0, 255), Colour::bgr2hsv(white) );
}
