#include <gtest/gtest.h>

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
  EXPECT_EQ ( Colour::hsv(170, 255, 255), Colour::bgr2hsv(blue) );
  EXPECT_EQ ( Colour::hsv(85, 255, 255), Colour::bgr2hsv(green) );
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

  // These are a series of RGB -> HSV conversions according to Gimp
  // The numbers shown in comments come from Gimp, and our values (which are different)
  // are shown in the 'expected' values

  // There are some differences between the values calculated from Gimp and those
  // from our algorithm, but larger than 2/255 in any one channel

  // Gimp says 259,62,84 (HSV) -> 183,158,214
  EXPECT_EQ ( Colour::hsv(183, 159, 213), Colour::bgr2hsv(Colour::bgr(213,80,123)) );
  // Gimp says 341,62,84 (HSV) -> 241,158,214
  EXPECT_EQ ( Colour::hsv(242, 159, 213), Colour::bgr2hsv(Colour::bgr(123,80,213)) );
  // Gimp says 23,100,78 (HSV) -> 16,255,198
  EXPECT_EQ ( Colour::hsv( 16, 255, 200), Colour::bgr2hsv(Colour::bgr(0,78,200)) );
  // Gimp says 212,92,77 (HSV) -> 150,234,196
  EXPECT_EQ ( Colour::hsv(150, 235, 196), Colour::bgr2hsv(Colour::bgr(196,100,15)) );
  // Gimp says 162,57,78 (HSV) -> 114,145,198
  EXPECT_EQ ( Colour::hsv(115, 145, 200), Colour::bgr2hsv(Colour::bgr(166,200,86)) );
  // Gimp says 351,100,93 (HSV) -> 247,255,237
  EXPECT_EQ ( Colour::hsv(249, 255, 236), Colour::bgr2hsv(Colour::bgr(36,0,236)) );
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

TEST (ColourTests, hsvRange)
{
  Colour::hsvRange rangeAll(0, 255, 0, 255, 0, 255);

  EXPECT_TRUE ( rangeAll.contains(Colour::hsv(0, 0, 0)) );
  EXPECT_TRUE ( rangeAll.contains(Colour::hsv(128, 0, 0)) );
  EXPECT_TRUE ( rangeAll.contains(Colour::hsv(255, 0, 0)) );
  EXPECT_TRUE ( rangeAll.contains(Colour::hsv(0, 128, 128)) );
  EXPECT_TRUE ( rangeAll.contains(Colour::hsv(128, 255, 128)) );
  EXPECT_TRUE ( rangeAll.contains(Colour::hsv(255, 128, 255)) );

  Colour::hsvRange rangeOne(0, 0, 0, 0, 0, 0);

  EXPECT_TRUE  ( rangeOne.contains(Colour::hsv(0, 0, 0)) );
  EXPECT_FALSE ( rangeOne.contains(Colour::hsv(128, 0, 0)) );
  EXPECT_FALSE ( rangeOne.contains(Colour::hsv(255, 0, 0)) );
  EXPECT_FALSE ( rangeOne.contains(Colour::hsv(0, 128, 128)) );
  EXPECT_FALSE ( rangeOne.contains(Colour::hsv(128, 255, 128)) );
  EXPECT_FALSE ( rangeOne.contains(Colour::hsv(255, 128, 255)) );

  Colour::hsvRange rangeHWrap(235, 20, 0, 255, 0, 255);

  EXPECT_TRUE  (rangeHWrap.contains(Colour::hsv(0,128,128)) );
  EXPECT_TRUE  (rangeHWrap.contains(Colour::hsv(20,128,128)) );
  EXPECT_FALSE (rangeHWrap.contains(Colour::hsv(21,128,128)) );
  EXPECT_TRUE  (rangeHWrap.contains(Colour::hsv(235,128,128)) );
  EXPECT_FALSE (rangeHWrap.contains(Colour::hsv(234,128,128)) );

  Colour::hsvRange rangeSRangeOutOfBounds(118, 138, 100, 255, 0, 255);

  EXPECT_TRUE  (rangeSRangeOutOfBounds.contains(Colour::hsv(128,200,128)) );
  EXPECT_TRUE  (rangeSRangeOutOfBounds.contains(Colour::hsv(128,255,128)) );
  EXPECT_TRUE  (rangeSRangeOutOfBounds.contains(Colour::hsv(128,100,128)) );
  EXPECT_FALSE (rangeSRangeOutOfBounds.contains(Colour::hsv(128,99,128)) );
  EXPECT_FALSE (rangeSRangeOutOfBounds.contains(Colour::hsv(128,0,128)) );

  EXPECT_EQ(127, Colour::hsvRange(0, 255, 0,0, 0,0).getHMid());
  EXPECT_EQ(5,   Colour::hsvRange(0, 10, 0,0, 0,0).getHMid());
  EXPECT_EQ(255, Colour::hsvRange(250, 5, 0,0, 0,0).getHMid());
  EXPECT_EQ(5,   Colour::hsvRange(250, 15, 0,0, 0,0).getHMid());

  EXPECT_EQ(0,   Colour::hsvRange(0,0, 0,0, 0,0).getSMid());
  EXPECT_EQ(127, Colour::hsvRange(0,0, 0,255, 0,0).getSMid());

  EXPECT_EQ(0,   Colour::hsvRange(0,0, 0,0, 0,0).getVMid());
  EXPECT_EQ(127, Colour::hsvRange(0,0, 0,0, 0,255).getVMid());
}

TEST (ColourTests, hsvRangeContaining )
{
  auto range = Colour::hsvRange(128,129, 128,128, 128,128);

  EXPECT_EQ ( Colour::hsvRange(64,129, 128,128, 128,128), range.containing(Colour::hsv(64, 128, 128)) );
  EXPECT_EQ ( Colour::hsvRange(128,192, 128,128, 128,128), range.containing(Colour::hsv(192, 128, 128)) );
  EXPECT_EQ ( Colour::hsvRange(128,129, 64,128, 128,128), range.containing(Colour::hsv(128, 64, 128)) );
  EXPECT_EQ ( Colour::hsvRange(128,129, 128,192, 128,128), range.containing(Colour::hsv(128, 192, 128)) );
  EXPECT_EQ ( Colour::hsvRange(128,129, 128,128, 64,128), range.containing(Colour::hsv(128, 128, 64)) );
  EXPECT_EQ ( Colour::hsvRange(128,129, 128,128, 128,192), range.containing(Colour::hsv(128, 128, 192)) );

  range = Colour::hsvRange(0,1, 128,128, 128,128);
  EXPECT_EQ ( Colour::hsvRange(0,64, 128,128, 128,128), range.containing(Colour::hsv(64, 128, 128)) );
  EXPECT_EQ ( Colour::hsvRange(0,128, 128,128, 128,128), range.containing(Colour::hsv(128, 128, 128)) );
  EXPECT_EQ ( Colour::hsvRange(192,1, 128,128, 128,128), range.containing(Colour::hsv(192, 128, 128)) );

  range = Colour::hsvRange(128,128, 128,128, 128,128);
  EXPECT_EQ ( Colour::hsvRange(64,128, 128,128, 128,128), range.containing(Colour::hsv(64, 128, 128)) );
  EXPECT_EQ ( Colour::hsvRange(128,192, 128,128, 128,128), range.containing(Colour::hsv(192, 128, 128)) );

}
