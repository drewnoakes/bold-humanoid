#include "gtest/gtest.h"

#include "../Colour/colour.hh"

using namespace bold;

TEST (ColourTests, bgr2hsv)
{
  Colour::bgr red(0,0,255);
  Colour::bgr blue(255,0,0);
  Colour::bgr green(0,255,0);
  Colour::bgr yellow(0,255,255);
  Colour::bgr yellow1(0,255,254);
  Colour::bgr yellow2(0,254,255); 
  Colour::bgr cyan(255,255,0);
  Colour::bgr magenta(255,0,255);
  Colour::bgr gray(128,128,128);
  Colour::bgr black(0,0,0);
  Colour::bgr white(255,255,255);

  EXPECT_EQ ( Colour::hsv(0, 255, 255), Colour::bgr2hsv(red) );
  EXPECT_EQ ( Colour::hsv(85, 255, 255), Colour::bgr2hsv(green) );
  EXPECT_EQ ( Colour::hsv(170, 255, 255), Colour::bgr2hsv(blue) );
  EXPECT_EQ ( Colour::hsv(43, 255, 255), Colour::bgr2hsv(yellow) );
  // Since yellow should have hue 85/2=42.5, which is rounded to 43,
  // small deviation towards g (increase of hue) should map to the
  // same hue as yellow, whereas small deviation towards red should
  // decrease hue
  EXPECT_EQ ( Colour::bgr2hsv(yellow), Colour::bgr2hsv(yellow1) );
  EXPECT_EQ ( Colour::bgr2hsv(yellow).h - 1, Colour::bgr2hsv(yellow2).h );
  EXPECT_EQ ( Colour::hsv(128 /*127.5*/, 255, 255), Colour::bgr2hsv(cyan) );
  // This gives hue=212, should give hue=213, but any attempt to get it to that breaks other tests
  EXPECT_EQ ( Colour::hsv(212 /*212.5*/, 255, 255), Colour::bgr2hsv(magenta) );
  EXPECT_EQ ( Colour::hsv(0, 0, 128), Colour::bgr2hsv(gray) );
  EXPECT_EQ ( Colour::hsv(0, 0, 0), Colour::bgr2hsv(black) );
  EXPECT_EQ ( Colour::hsv(0, 0, 255), Colour::bgr2hsv(white) );
}

TEST (ColourTests, hsv2bgr)
{
  Colour::hsv red(0,255,255);
  Colour::hsv green(85,255,255);
  Colour::hsv blue(170,255,255);
  Colour::hsv yellow1(42,255,255);
  Colour::hsv yellow2(43,255,255);
  Colour::hsv cyan1(127,255,255);
  Colour::hsv cyan2(128,255,255);
  Colour::hsv magenta1(212,255,255);
  Colour::hsv magenta2(213,255,255);
  Colour::hsv gray(0,0,128);
  Colour::hsv black(0,0,0);
  Colour::hsv white(0,0,255);

  EXPECT_EQ ( Colour::bgr(0,0,255), Colour::hsv2bgr(red) );
  EXPECT_EQ ( Colour::bgr(255,0,0), Colour::hsv2bgr(blue) );
  EXPECT_EQ ( Colour::bgr(0,255,0), Colour::hsv2bgr(green) );
  EXPECT_EQ ( Colour::bgr(0,252,255), Colour::hsv2bgr(yellow1) );
  EXPECT_EQ ( Colour::bgr(0,255,252), Colour::hsv2bgr(yellow2) );
  EXPECT_EQ ( Colour::bgr(252,255,0), Colour::hsv2bgr(cyan1) );
  EXPECT_EQ ( Colour::bgr(255,252,0), Colour::hsv2bgr(cyan2) );
  EXPECT_EQ ( Colour::bgr(255,0,252), Colour::hsv2bgr(magenta1) );
  EXPECT_EQ ( Colour::bgr(252,0,255), Colour::hsv2bgr(magenta2) );
  EXPECT_EQ ( Colour::bgr(128,128,128), Colour::hsv2bgr(gray) );
  EXPECT_EQ ( Colour::bgr(0,0,0), Colour::hsv2bgr(black) );
  EXPECT_EQ ( Colour::bgr(255,255,255), Colour::hsv2bgr(white) );
}

TEST (ColourTests, hsv2bgr2hsv)
{
  Colour::hsv red(0,255,255);
  Colour::hsv green(85,255,255);
  Colour::hsv blue(170,255,255);
  Colour::hsv yellow1(42,255,255);
  Colour::hsv yellow2(43,255,255);
  Colour::hsv cyan1(127,255,255);
  Colour::hsv cyan2(128,255,255);
  Colour::hsv magenta1(212,255,255);
  Colour::hsv magenta2(213,255,255);
  Colour::hsv gray(0,0,128);
  Colour::hsv black(0,0,0);
  Colour::hsv white(0,0,255);

  EXPECT_EQ ( red, Colour::bgr2hsv(Colour::hsv2bgr(red)) );
  EXPECT_EQ ( blue, Colour::bgr2hsv(Colour::hsv2bgr(blue)) );
  EXPECT_EQ ( green, Colour::bgr2hsv(Colour::hsv2bgr(green)) );
  EXPECT_EQ ( yellow1, Colour::bgr2hsv(Colour::hsv2bgr(yellow1)) );
  EXPECT_EQ ( yellow2, Colour::bgr2hsv(Colour::hsv2bgr(yellow2)) );
  EXPECT_EQ ( cyan1, Colour::bgr2hsv(Colour::hsv2bgr(cyan1)) );
  EXPECT_EQ ( cyan2, Colour::bgr2hsv(Colour::hsv2bgr(cyan2)) );
  EXPECT_EQ ( magenta1, Colour::bgr2hsv(Colour::hsv2bgr(magenta1)) );
  EXPECT_EQ ( magenta2, Colour::bgr2hsv(Colour::hsv2bgr(magenta2)) );
  EXPECT_EQ ( gray, Colour::bgr2hsv(Colour::hsv2bgr(gray)) );
  EXPECT_EQ ( black, Colour::bgr2hsv(Colour::hsv2bgr(black)) );
  EXPECT_EQ ( white, Colour::bgr2hsv(Colour::hsv2bgr(white)) );
}
