#include "gtest/gtest.h"
#include "../HistogramLabelTeacher/histogramlabelteacher.hh"

#include <opencv2/opencv.hpp>
#include <vector>

using namespace std;
using namespace bold;

TEST (HistogramLabelTeacherTests, init)
{
  auto names = vector<string>{string{"one"}, string{"two"}};
  HistogramLabelTeacher<6> teacher{names};

  auto labels = teacher.getLabels();
  
  EXPECT_EQ(2, labels.size());
}

TEST (HistogramLabelTeacherTests, setTrainImage)
{
  cv::Mat trainImage{640, 480, CV_8UC3};

  auto names = vector<string>{string{"one"}, string{"two"}};
  HistogramLabelTeacher<6> teacher{names};

  teacher.setTrainImage(trainImage);
}

TEST (HistogramLabelTeacherTests, setSeedPoint)
{
  auto names = vector<string>{string{"one"}, string{"two"}};
  HistogramLabelTeacher<6> teacher{names};

  auto point = Eigen::Vector2i{320, 240};

  teacher.setSeedPoint(point);
}

TEST (HistogramLabelTeacherTests, floodFillEmpty)
{
  auto names = vector<string>{string{"one"}, string{"two"}};
  HistogramLabelTeacher<6> teacher{names};

  cv::Mat trainImage = cv::Mat::zeros(640, 480, CV_8UC3);
  teacher.setTrainImage(trainImage);
  teacher.setSeedPoint(Eigen::Vector2i{0, 0});

  auto mask = teacher.floodFill();

  EXPECT_EQ ( trainImage.rows, mask.rows );
  EXPECT_EQ ( trainImage.cols, mask.cols );

  cout << mask.type() << endl;

  for (unsigned i = 0; i < mask.rows; ++i)
    for (unsigned j = 0; j < mask.cols; ++j)
      EXPECT_EQ ( 255, mask.at<uint8_t>(i, j) ) << i << " " << j;
}

TEST (HistogramLabelTeacherTests, floodFillColor)
{
  auto names = vector<string>{string{"one"}, string{"two"}};
  HistogramLabelTeacher<6> teacher{names};

  cv::Mat trainImage = cv::Mat::zeros(640, 480, CV_8UC3);
  cv::rectangle(trainImage, cv::Rect(0, 0, 100, 100), cv::Scalar(255, 0, 0), CV_FILLED);
  teacher.setTrainImage(trainImage);
  teacher.setSeedPoint(Eigen::Vector2i{0, 0});

  auto mask = teacher.floodFill();
  for (unsigned i = 0; i < mask.rows; ++i)
    for (unsigned j = 0; j < mask.cols; ++j)
      if (i < 100 && j < 100)
        EXPECT_EQ ( 255, mask.at<uint8_t>(i, j) ) << i << " " << j;
      else
        EXPECT_EQ ( 0, mask.at<uint8_t>(i, j) ) << i << " " << j;
}

TEST (HistogramLabelTeacherTests, floodFillNoColor)
{
  auto names = vector<string>{string{"one"}, string{"two"}};
  HistogramLabelTeacher<6> teacher{names};

  cv::Mat trainImage = cv::Mat::zeros(640, 480, CV_8UC3);
  cv::rectangle(trainImage, cv::Rect(0, 0, 100, 100), cv::Scalar(255, 0, 0), CV_FILLED);
  teacher.setTrainImage(trainImage);
  teacher.setSeedPoint(Eigen::Vector2i{320, 240});

  auto mask = teacher.floodFill();
  for (unsigned i = 0; i < mask.rows; ++i)
    for (unsigned j = 0; j < mask.cols; ++j)
      if (i < 100 && j < 100)
        EXPECT_EQ ( 0, mask.at<uint8_t>(i, j) ) << i << " " << j;
      else
        EXPECT_EQ ( 255, mask.at<uint8_t>(i, j) ) << i << " " << j;
}

TEST (HistogramLabelTeacherTests, train)
{
  auto names = vector<string>{string{"one"}};
  HistogramLabelTeacher<6> teacher{names};

  cv::Mat trainImage = cv::Mat::zeros(640, 480, CV_8UC3);
  cv::rectangle(trainImage, cv::Rect(0, 0, 100, 100), cv::Scalar(128, 0, 0), CV_FILLED);
  teacher.setTrainImage(trainImage);
  teacher.setSeedPoint(Eigen::Vector2i{0, 0});

  auto mask = teacher.floodFill();

  teacher.train("one", mask);

  auto labels = teacher.getLabels();

  auto labelOne = labels[0];

  EXPECT_EQ ( 100 * 100, labelOne.getTotalCount() );

  EXPECT_EQ ( Colour::hsv(128, 0, 0), labelOne.modalColour() );

  EXPECT_EQ ( 1.0f, labelOne.labelProb(Colour::hsv(128, 0, 0)) );
  EXPECT_EQ ( 0.0f, labelOne.labelProb(Colour::hsv(0, 0, 0)) );
  EXPECT_EQ ( 0.0f, labelOne.labelProb(Colour::hsv(255, 0, 0)) );

}
