#include "gtest/gtest.h"

#include "../PixelLabel/HistogramPixelLabel/histogrampixellabel.hh"

#include <fstream>

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

TEST (HistogramPixelLabelTests, empty)
{
  HistogramPixelLabel<8> label("label");

  EXPECT_EQ ( 1.0f / (256 * 256 * 256), label.labelProb(Colour::hsv(0, 0, 0)) );
}

TEST (HistogramPixelLabelTests, readWrite)
{
  HistogramPixelLabel<6> label1("label");
  for (unsigned i = 0; i < 100; ++i)
    label1.addSample(Colour::hsv(128, 64, 224));

  ofstream out("/tmp/label.dat");
  label1.write(out);
  out.close();

  HistogramPixelLabel<6> label2("label");
  ifstream in("/tmp/label.dat");
  label2.read(in);
  in.close();

  EXPECT_EQ ( label1.getTotalCount(), label2.getTotalCount() );
  EXPECT_EQ ( label1.modalColour(), label2.modalColour() );

}

