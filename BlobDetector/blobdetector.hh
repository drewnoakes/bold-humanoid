#ifndef BOLD_BLOBDETECTOR_HH
#define BOLD_BLOBDETECTOR_HH

#include <Eigen/Core>
#include <opencv.hpp>
#include "../DisjointSet/disjointset.hh"

namespace bold
{
  /** Horizontal run of pixels
   */
  struct Run
  {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Run(unsigned x, unsigned y)
      : start(x, y),
        end(x+1, y),
        length(1)
    {}

    Eigen::Vector2i start;   ///< Start pixel
    Eigen::Vector2i end;     ///< End pixel
    unsigned length;         ///< Number of pixels

    /** Compare operator
     *
     * Orders runs first by y, then by x of start
     */
    bool operator<(Run const& other) const
    {
      return
        start.y() < other.start.y() ||
        (start.y() == other.start.y() && start.x() < other.start.x());
    }
  };

  struct Blob
  {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Eigen::Vector2i ul;      ///< Upper left pixel
    Eigen::Vector2i br;      ///< Bottom righ pixel
    unsigned area;           ///< Number of pixes in blob
    Eigen::Vector2f mean;    ///< Mean
    Eigen::Matrix2f covar;   ///< Covarience

    std::set<Run> runs;      ///< Runs in this blob

    bool operator<(Blob const& other) const
    {
      return
        area < other.area ||
        (area == other.area && mean.y() < other.mean.y());
    }
  };

  class BlobDetector
  {
  public:
    std::vector<std::set<Blob > > detectBlobs(cv::Mat const& labeledImage, unsigned char nLabels);

  private:
    typedef std::vector<std::vector<Run>> RunLengthCode;

    std::vector<RunLengthCode> runLengthEncode(cv::Mat const& labeledImage, unsigned char nLabels);

    static Blob runSetToBlob(std::set<Run> const& runSet);
  };
}

#endif
