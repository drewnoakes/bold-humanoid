#ifndef BATS_BLOBDETECTOR_HH
#define BATS_BLOBDETECTOR_HH

#include <opencv.hpp>

namespace bats
{
  struct Blob
  {
    Eigen::Vector2u ul;
    Eigen::Vector2u br;
    unsigned area;
    Eigen::Vector2u mu;
    Eigen::Matrix2u covar;
  };

  struct Run
  {
    Run(unsigned x, unsigned y)
      : start(x, y),
	end(x+1, y),
	length(1)
    {}

    Eigen::Vector2u start;
    Eigen::Vector2u end;
    unsigned length;
  };

  class BlobDetector
  {
  public:
    void detectBlobs(cv::Mat const& labeledImage, unsigned nLabels);

  private:
    typedef std::vector<std::vector<Run>> RunLengthCode;
    std::vector<RunLengthCode> runLengthEncode(cv::Mat const& labeledImage, unsigned nLabels);
  };
}

#endif
