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

    Run(unsigned startX, unsigned y)
      : startX(startX),
        endX(startX),
        y(y)
    {}

    unsigned y;        ///< The row index of the horizontal run
    unsigned startX;   ///< The column index of the left-most pixel
    unsigned endX;     ///< The column index of the right-most pixel

    /** Returns the number of pixels in the run.
     *
     * If a run starts and stops at the same x position, it has length one.
     */
    inline unsigned length() const
    {
      return endX - startX + 1;
    }

    /** Returns whether two runs overlap, ignoring y positions. */
    inline static bool overlaps(Run const& a, Run const& b)
    {
      return std::max(a.endX, b.endX) - std::min(a.startX, b.startX) < a.length() + b.length();
    }

    /** Compare operator
     *
     * Orders runs by y, then by startX.
     */
    bool operator<(Run const& other) const
    {
      return
        y < other.y ||
       (y == other.y && startX < other.startX);
    }

    friend std::ostream& operator<<(std::ostream& stream, Run const& run)
    {
      return stream << "Run (y=" << run.y << " x=[" << run.startX << "," << run.endX << "] len=" << run.length() << ")";
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

    Blob()
    : ul(1e6,1e6),
      br(-1,-1),
      area(0)
    {
      // can these be done in the initialiser list?
      mean << 0, 0;
      covar << 0, 0, 0, 0;
    }

    cv::Rect toRect() const
    {
      auto size = br - ul;
      return cv::Rect(ul.x(), ul.y(), size.x(), size.y());
    }

    bool operator<(Blob const& other) const
    {
      return
        area > other.area ||
        (area == other.area && mean.y() < other.mean.y());
    }
  };

  typedef std::function<bool(Run const& a, Run const& b)> UnionPredicate;

  /** Specifies a unit of work for blob detection. Contains the pixel label and union predicate function. */
  struct BlobType
  {
    const PixelLabel pixelLabel;
    const UnionPredicate unionPredicate;

    BlobType(PixelLabel pixelLabel, UnionPredicate unionPredicate)
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

    std::vector<BlobType> d_blobTypes;
    /** Accumulated data for the most recently passed image. */
    std::map<uchar, RunLengthCode> d_runsPerRowPerLabel;
    bold::Run d_currentRun;
    uchar d_currentLabel;

    /** Processes Runs into Blobs. Returns a set of blobs per label. */
    std::map<bold::PixelLabel,std::set<Blob>> detectBlobs();

    static Blob runSetToBlob(std::set<Run> const& runSet);

    void addRun(int endX)
    {
      assert(endX >= d_currentRun.startX);

      // finish whatever run we were on
      d_currentRun.endX = endX;

      // TODO do this with pointer arithmetic rather than a map lookup
      auto it = d_runsPerRowPerLabel.find(d_currentLabel);
      if (it != d_runsPerRowPerLabel.end())
      {
        it->second[d_currentRun.y].push_back(d_currentRun);
      }
    }

  public:
    /** The collection of blobs found during the last image pass. */
    std::map<bold::PixelLabel,std::set<Blob>> blobsPerLabel;

    BlobDetectPass(int imageWidth, int imageHeight, std::vector<BlobType> blobTypes)
    : d_blobTypes(blobTypes),
      d_imageHeight(imageHeight),
      d_imageWidth(imageWidth),
      d_runsPerRowPerLabel(),
      d_currentRun(0, 0)
    {
      // Create a run length code for each label
      for (BlobType const& blobType : blobTypes)
      {
        uchar pixelLabelId = blobType.pixelLabel.id();

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
      for (auto& pair : d_runsPerRowPerLabel)
      {
        auto& runsPerRow = pair.second;
        for (std::vector<bold::Run>& runs : runsPerRow)
        {
          runs.clear();
        }
      }
    }

    void onRowStarting(int y)
    {
      // TODO might miss last run on last row with this approach -- add onRowEnding, or copy into onImageComplete
      if (d_currentLabel != 0)
      {
        // finish whatever run we were on
        addRun(d_imageWidth - 1);
      }
      d_currentRun.y = y;
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
          assert(x > 0);
          addRun(x - 1);
        }

        // Check whether this is the start of a new run
        if (label != 0)
        {
          // Start new run
          d_currentRun.startX = x;
        }

        d_currentLabel = label;
      }
    }

    void onImageComplete()
    {
      blobsPerLabel = detectBlobs();
    }

    std::vector<BlobType> blobTypes() const { return d_blobTypes; }
  };
}

#endif
