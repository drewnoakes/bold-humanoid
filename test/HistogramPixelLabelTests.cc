#include "gtest/gtest.h"

#include "../PixelLabel/HistogramPixelLabel/histogrampixellabel.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

TEST (HistogramPixelLabelTests, binSize)
{
  HistogramPixelLabel<8> label1("label1");
  EXPECT_EQ ( 1, label1.getBinSize() );
  EXPECT_EQ ( 256, label1.getNBins() );

  HistogramPixelLabel<7> label2("label2");
  EXPECT_EQ ( 2, label2.getBinSize() );
  EXPECT_EQ ( 128, label2.getNBins() );

  HistogramPixelLabel<6> label3("label3");
  EXPECT_EQ ( 4, label3.getBinSize() );
  EXPECT_EQ ( 64, label3.getNBins() );

  HistogramPixelLabel<1> label4("label4");
  EXPECT_EQ ( 128, label4.getBinSize() );
  EXPECT_EQ ( 2, label4.getNBins() );
}

TEST (HistogramPixelLabelTests, index)
{
  HistogramPixelLabel<8> label("label");
  EXPECT_EQ ( Colour::hsv(0, 0, 0), label.indexToHsv(0) );
  EXPECT_EQ ( Colour::hsv(0, 0, 1), label.indexToHsv(1) );
  EXPECT_EQ ( Colour::hsv(0, 1, 0), label.indexToHsv(256) );
  EXPECT_EQ ( Colour::hsv(0, 1, 1), label.indexToHsv(257) );
  EXPECT_EQ ( Colour::hsv(1, 0, 0), label.indexToHsv(65536) );

  EXPECT_EQ ( 0, label.hsvToIndex(0, 0, 0) );
  EXPECT_EQ ( 1, label.hsvToIndex(0, 0, 1) );
  EXPECT_EQ ( 256, label.hsvToIndex(0, 1, 0) );
  EXPECT_EQ ( 257, label.hsvToIndex(0, 1, 1) );
  EXPECT_EQ ( 65536, label.hsvToIndex(1, 0, 0) );
}
