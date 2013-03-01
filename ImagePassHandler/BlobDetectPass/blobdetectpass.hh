#ifndef BOLD_BLOB_DETECT_PASS_HH
#define BOLD_BLOB_DETECT_PASS_HH

#include <opencv2/core/core.hpp>
#include <Eigen/Core>
#include <algorithm>
#include <vector>
#include <set>
#include <cassert>

#include "../imagepasshandler.hh"
#include "../../DisjointSet/disjointset.hh"
#include "../../PixelLabel/pixellabel.hh"

namespace bold
{
  /** Horizontal run of pixels */
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

  /** A conjoined set of Runs */
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
        area > other.area ||
        (area == other.area && mean.y() < other.mean.y());
    }
  };

  typedef std::function<bool(Run const& a, Run const& b)> UnionPredicate;

  /** Specifies a unit of work for blob detection. Contains the pixel label and union predicate function. */
  struct BlobLabel
  {
    const PixelLabel pixelLabel;
    const UnionPredicate unionPredicate;

    BlobLabel(PixelLabel pixelLabel, UnionPredicate unionPredicate)
    : pixelLabel(pixelLabel),
      unionPredicate(unionPredicate)
    {}
  };

  class BlobDetectPass : public ImagePassHandler<uchar>
  {
  private:
    typedef std::vector<std::vector<Run>> RunLengthCode;

    int d_imageHeight;
    int d_imageWidth;

    std::vector<BlobLabel> d_blobLabels;
    /** Accumulated data for the most recently passed image. */
    std::map<uchar, RunLengthCode> d_runsPerRowPerLabel;
    bold::Run d_currentRun;
    uchar d_currentLabel;

    /** Processes Runs into Blobs. Returns a set of blobs per label. */
    std::map<bold::PixelLabel,std::set<Blob>> detectBlobs();

    static Blob runSetToBlob(std::set<Run> const& runSet);

    void addRun(int x, int y)
    {
      assert(y == d_currentRun.start.y());

      // finish whatever run we were on
      d_currentRun.end = Eigen::Vector2i(x, y);
      d_currentRun.length = x - d_currentRun.start.x();

      auto it = d_runsPerRowPerLabel.find(d_currentLabel);
      if (it != d_runsPerRowPerLabel.end())
      {
        it->second[y].push_back(d_currentRun);
      }
    }

  public:
    /** The collection of blobs found during the last image pass. */
    std::map<bold::PixelLabel,std::set<Blob>> blobsPerLabel;

    BlobDetectPass(int imageWidth, int imageHeight, std::vector<BlobLabel> blobLabels)
    : d_blobLabels(blobLabels),
      d_imageHeight(imageHeight),
      d_imageWidth(imageWidth),
      d_runsPerRowPerLabel(),
      d_currentRun(0, 0)
    {
      // Create a run length code for each label
      for (BlobLabel const& blobLabel : blobLabels)
      {
        uchar pixelLabelId = blobLabel.pixelLabel.id();

        // A RunLengthCode is a vector of vectors of runs
        d_runsPerRowPerLabel[pixelLabelId] = RunLengthCode();

        // Initialise a vector of Runs for each row in the image
        for (unsigned y = 0; y < d_imageHeight; ++y)
          d_runsPerRowPerLabel[pixelLabelId].push_back(std::vector<bold::Run>());
      }
    }

    void onImageStarting()
    {
      // Clear all persistent data
      for (auto pair : d_runsPerRowPerLabel)
      {
        auto runsPerRow = pair.second;
        for (std::vector<bold::Run> runs : runsPerRow)
        {
          runs.clear();
        }
      }
    }

    void onRowStarting(int y)
    {
      if (d_currentLabel != 0)
      {
        // finish whatever run we were on
        addRun(d_imageWidth - 1, d_currentRun.start.y());
      }
      d_currentRun = bold::Run(0, 0);
      d_currentLabel = 0;
    }

    void onPixel(uchar label, int x, int y)
    {
      // Check if we have a run boundary
      if (label != d_currentLabel)
      {
        // Check whether this is the end of the current run
        if (d_currentLabel != 0)
        {
          // Finished run
          addRun(x, y);
        }

        // Check whether this is the start of a new run
        if (label != 0)
        {
          // Start new run
          d_currentRun = Run(x, y);
        }

        d_currentLabel = label;
      }
    }

    void onImageComplete()
    {
      blobsPerLabel = detectBlobs();
    }
  };
}

#endif