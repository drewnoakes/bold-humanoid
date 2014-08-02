#include "gtest/gtest.h"
#include "helpers.hh"
#include "../util/windowfunctions.hh"

using namespace std;
using namespace bold;

TEST (WindowFunctionTests, rectangle)
{
  vector<double> window(5);
  WindowFunction::rectangle(window);

  EXPECT_EQ(1.0, window[0]);
  EXPECT_EQ(1.0, window[1]);
  EXPECT_EQ(1.0, window[2]);
  EXPECT_EQ(1.0, window[3]);
  EXPECT_EQ(1.0, window[4]);
}

TEST (WindowFunctionTests, triangular)
{
  vector<double> window(5);
  WindowFunction::triangular(window);

  EXPECT_EQ(0.0, window[0]);
  EXPECT_EQ(0.5, window[1]);
  EXPECT_EQ(1.0, window[2]);
  EXPECT_EQ(0.5, window[3]);
  EXPECT_EQ(0.0, window[4]);
}
