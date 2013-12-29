#include "integralimage.hh"

using namespace bold;
using namespace cv;
using namespace Eigen;
using namespace std;

IntegralImage IntegralImage::create(cv::Mat const& image)
{
  assert(image.rows > 0 && image.cols > 0 && "Must have at least one row and one column");
  assert(image.type() == CV_8UC1 && "Image must be of type CV_8UC1");

  cv::Mat integral(image.size(), CV_32SC1);

  // Process the first row
  uchar const* source = image.ptr<uchar const>(0);
  int* target = integral.ptr<int>(0);
  int sum = 0;
  for (int x = 0; x < image.cols; x++)
  {
    sum += *(source++);
    *(target++) = sum;
  }

  // Subsequent rows consider the row above as well
  for (int y = 1; y < image.rows; y++)
  {
    source = image.ptr<uchar const>(y);
    target = integral.ptr<int>(y);
    int const* above = integral.ptr<int const>(y - 1);

    *target = *source + *above;

    for (int x = 1; x < image.cols; x++)
    {
      int leftValue = *(target++);
      int aboveLeftValue = *(above++);

      *target = *(source++) + leftValue + *above - aboveLeftValue;
    }
  }

  return IntegralImage(integral);
}

int IntegralImage::getSummedArea(Eigen::Vector2i min, Eigen::Vector2i max) const
{
  int sum = at(max.x(), max.y());

  if (min.x() != 0 && min.y() != 0)
  {
    sum += at(min.x() - 1, min.y() - 1);
    sum -= at(min.x() - 1, max.y());
    sum -= at(max.x(), min.y() - 1);
  }
  else if (min.y() != 0)
  {
    sum -= at(max.x(), min.y() - 1);
  }
  else if (min.x() != 0)
  {
    sum -= at(min.x() - 1, max.y());
  }

  return sum;
}
