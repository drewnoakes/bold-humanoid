#ifndef BATS_BLOBDETECTOR_HH
#define BATS_BLOBDETECTOR_HH

#include <Eigen/Core>
#include <opencv.hpp>
#include "../DisjointSet/disjointset.hh"

namespace bats
{
  struct Blob
  {
    Eigen::Vector2i ul;
    Eigen::Vector2i br;
    unsigned area;
    Eigen::Vector2i mu;
    Eigen::Matrix2i covar;
  };

  struct Run
  {
    Run(unsigned x, unsigned y)
      : start(x, y),
	end(x+1, y),
	length(1)
    {}

    Eigen::Vector2i start;
    Eigen::Vector2i end;
    unsigned length;

    bool operator<(Run const& other) const
    {
      return
	start.y() < other.start.y() ||
	(start.y() == other.start.y() && start.x() < other.start.x());
    }
  };

  class BlobDetector
  {
  public:
    std::vector<std::set<std::set<Run> > > detectBlobs(cv::Mat const& labeledImage, unsigned nLabels);

  private:
    typedef std::vector<std::vector<Run>> RunLengthCode;
    std::vector<RunLengthCode> runLengthEncode(cv::Mat const& labeledImage, unsigned nLabels);
  };
}

#endif
