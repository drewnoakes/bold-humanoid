#include <gtest/gtest.h>
#include "../LabelTeacher/labelteacher.hh"
#include "../PixelLabel/HistogramPixelLabel/histogrampixellabel.hh"
#include <opencv2/opencv.hpp>
#include <vector>

using namespace std;
using namespace bold;

TEST (LabelTeacherTests, init)
{
  auto labels = vector<shared_ptr<PixelLabel>>{
    make_shared<HistogramPixelLabel<6>>("one", LabelClass::BALL),
    make_shared<HistogramPixelLabel<6>>("two", LabelClass::FIELD)
  };
  LabelTeacher teacher{labels};

  auto _labels = teacher.getLabels();

  EXPECT_EQ(2, _labels.size());
}

TEST (LabelTeacherTests, setYUVTrainImage)
{
  cv::Mat trainImage{640, 480, CV_8UC3};

  auto labels = vector<shared_ptr<PixelLabel>>{
    make_shared<HistogramPixelLabel<6>>("one", LabelClass::BALL),
    make_shared<HistogramPixelLabel<6>>("two", LabelClass::FIELD)
  };
  LabelTeacher teacher{labels};

  teacher.setYUVTrainImage(trainImage);
}

TEST (LabelTeacherTests, setSeedPoint)
{
  auto labels = vector<shared_ptr<PixelLabel>>{
    make_shared<HistogramPixelLabel<6>>("one", LabelClass::BALL),
    make_shared<HistogramPixelLabel<6>>("two", LabelClass::FIELD)
  };
  LabelTeacher teacher{labels};

  auto point = Eigen::Vector2i{320, 240};

  teacher.setSeedPoint(point);
}

TEST (LabelTeacherTests, floodFillEmpty)
{
  auto labels = vector<shared_ptr<PixelLabel>>{
    make_shared<HistogramPixelLabel<6>>("one", LabelClass::BALL),
    make_shared<HistogramPixelLabel<6>>("two", LabelClass::FIELD)
  };
  LabelTeacher teacher{labels};

  cv::Mat trainImage = cv::Mat::zeros(640, 480, CV_8UC3);
  teacher.setYUVTrainImage(trainImage);
  teacher.setSeedPoint(Eigen::Vector2i{0, 0});

  auto mask = teacher.floodFill();

  EXPECT_EQ ( trainImage.rows, mask.rows );
  EXPECT_EQ ( trainImage.cols, mask.cols );

  cout << mask.type() << endl;

  for (int i = 0; i < mask.rows; ++i)
    for (int j = 0; j < mask.cols; ++j)
      EXPECT_EQ ( 255, mask.at<uint8_t>(i, j) ) << i << " " << j;
}

TEST (LabelTeacherTests, floodFillColor)
{
  auto labels = vector<shared_ptr<PixelLabel>>{
    make_shared<HistogramPixelLabel<6>>("one", LabelClass::BALL),
    make_shared<HistogramPixelLabel<6>>("two", LabelClass::FIELD)
  };
  LabelTeacher teacher{labels};

  cv::Mat trainImage = cv::Mat::zeros(640, 480, CV_8UC3);
  cv::rectangle(trainImage, cv::Rect(0, 0, 100, 100), cv::Scalar(255, 0, 0), CV_FILLED);
  teacher.setYUVTrainImage(trainImage);
  teacher.setSeedPoint(Eigen::Vector2i{0, 0});

  auto mask = teacher.floodFill();
  for (int i = 0; i < mask.rows; ++i)
    for (int j = 0; j < mask.cols; ++j)
      if (i < 100 && j < 100)
        EXPECT_EQ ( 255, mask.at<uint8_t>(i, j) ) << i << " " << j;
      else
        EXPECT_EQ ( 0, mask.at<uint8_t>(i, j) ) << i << " " << j;
}

TEST (LabelTeacherTests, floodFillNoColor)
{
  auto labels = vector<shared_ptr<PixelLabel>>{
    make_shared<HistogramPixelLabel<6>>("one", LabelClass::BALL),
    make_shared<HistogramPixelLabel<6>>("two", LabelClass::FIELD)
  };
  LabelTeacher teacher{labels};

  cv::Mat trainImage = cv::Mat::zeros(640, 480, CV_8UC3);
  cv::rectangle(trainImage, cv::Rect(0, 0, 100, 100), cv::Scalar(255, 0, 0), CV_FILLED);
  teacher.setYUVTrainImage(trainImage);
  teacher.setSeedPoint(Eigen::Vector2i{320, 240});

  auto mask = teacher.floodFill();
  for (int i = 0; i < mask.rows; ++i)
    for (int j = 0; j < mask.cols; ++j)
      if (i < 100 && j < 100)
        EXPECT_EQ ( 0, mask.at<uint8_t>(i, j) ) << i << " " << j;
      else
        EXPECT_EQ ( 255, mask.at<uint8_t>(i, j) ) << i << " " << j;
}

/*
TEST (LabelTeacherTests, DISABLED_train)
{
  auto labels = vector<shared_ptr<PixelLabel>>{
    make_shared<HistogramPixelLabel<6>>("one", LabelClass::BALL),
  };
  LabelTeacher teacher{labels};

  cv::Mat trainImage = cv::Mat::zeros(640, 480, CV_8UC3);
  cv::rectangle(trainImage, cv::Rect(0, 0, 100, 100), cv::Scalar(128, 0, 0), CV_FILLED);
  teacher.setYUVTrainImage(trainImage);
  teacher.setSeedPoint(Eigen::Vector2i{0, 0});

  auto mask = teacher.floodFill();

  teacher.train(0, mask);

  auto _labels = teacher.getLabels();

  auto labelOne = _labels[0];

  EXPECT_EQ ( 100 * 100, labelOne.getTotalCount() );

  EXPECT_EQ ( Colour::hsv(128, 0, 0), labelOne.modalColour() );

  EXPECT_EQ ( 1.0f, labelOne.labelProb(Colour::hsv(128, 0, 0)) );
  EXPECT_EQ ( 0.0f, labelOne.labelProb(Colour::hsv(0, 0, 0)) );
  EXPECT_EQ ( 0.0f, labelOne.labelProb(Colour::hsv(255, 0, 0)) );

}
*/
