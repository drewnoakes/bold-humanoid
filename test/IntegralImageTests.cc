#include "gtest/gtest.h"

#include "../IntegralImage/integralimage.hh"
#include <Eigen/Core>

using namespace bold;
using namespace std;
using namespace Eigen;

TEST (IntegralImageTests, create)
{
  int rows = 10;
  int cols = 12;
  cv::Mat image(cv::Size(cols, rows), CV_8UC1);

  image = cv::Scalar(1);

  for (int x = 0; x < cols; x++)
  {
    for (int y = 0; y < rows; y++)
      ASSERT_EQ(1, image.at<uchar>(y, x));
  }

  IntegralImage integral = IntegralImage::create(image);

  // 1 1 1 1
  // 1 1 1 1
  // 1 1 1 1

  // 1 2 3 4
  // 2 4 6 8
  // 3 6 9 12

  for (int y = 0; y < rows; y++)
  {
    for (int x = 0; x < cols; x++)
      ASSERT_EQ((x+1) * (y+1), integral.at(x, y)) << "Failed when x=" << x << ", y=" << y;
  }

  EXPECT_EQ ( 5*3, integral.getSummedArea(Vector2i(2, 3), Vector2i(6, 5)) );
  EXPECT_EQ ( 1, integral.getSummedArea(Vector2i(2, 3), Vector2i(2, 3)) );
  EXPECT_EQ ( rows*cols, integral.getSummedArea(Vector2i(0, 0), Vector2i(cols-1, rows-1)) );
  EXPECT_EQ ( 2*2, integral.getSummedArea(Vector2i(0, 2), Vector2i(1, 3)) );
  EXPECT_EQ ( 2*2, integral.getSummedArea(Vector2i(2, 0), Vector2i(3, 1)) );
  EXPECT_EQ ( 4*2, integral.getSummedArea(Vector2i(0, 0), Vector2i(3, 1)) );
  EXPECT_EQ ( 2*4, integral.getSummedArea(Vector2i(0, 0), Vector2i(1, 3)) );
}
