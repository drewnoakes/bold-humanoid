#pragma once

#include <opencv2/core/core.hpp>
#include <Eigen/Core>

#include <algorithm>
#include <memory>
#include <set>
#include <vector>
#include <Eigen/StdVector>

#include "../imagepasshandler.hh"
#include "../../DisjointSet/disjointset.hh"
#include "../../ImageLabelData/imagelabeldata.hh"
#include "../../geometry/Bounds.hh"
#include "../../PixelLabel/pixellabel.hh"
#include "../../util/assert.hh"
#include "../../SequentialTimer/sequentialtimer.hh"

namespace bold
{
  /** Horizontal run of pixels
   *
   **/
  struct Run
  {
    Run(unsigned startX, unsigned y);
    Run(unsigned startX, unsigned endX, unsigned y);

    /** Returns the number of pixels in the run.
     *
     * If a run starts and stops at the same x position, it has length one.
     **/
    unsigned length() const;

    /** Returns whether two runs overlap, ignoring y positions. */
    bool overlaps(Run const& b) const;

    /** Compare operator
     *
     * Orders runs by y, then by startX.
     */
    bool operator<(Run const& other) const;

    unsigned y;        ///< The row index of the horizontal run
    unsigned startX;   ///< The column index of the left-most pixel
    unsigned endX;     ///< The column index of the right-most pixel

    double midX() const { return (endX + startX) / 2.0; }

    inline friend std::ostream& operator<<(std::ostream& stream, Run const& run)
    {
      return stream << "Run (y=" << run.y << " x=[" << run.startX << "," << run.endX << "] len=" << run.length() << ")";
    }
  };

  /** A conjoined set of Runs
   *
   **/
  struct Blob
  {
    Blob();
    Blob(Eigen::Vector2i const& _ul, Eigen::Vector2i const& _br,
         unsigned _area,
         Eigen::Vector2d _mean, // Eigen::Matrix2d _covar,
         std::set<Run> const& _runs);

    cv::Rect toRect() const;

    Bounds2i bounds() const;

    void merge(Blob& other);

    bool operator<(Blob const& other) const;
    bool operator>(Blob const& other) const { return other < *this; }

    Eigen::Vector2i ul;      ///< Upper left pixel (min)
    Eigen::Vector2i br;      ///< Bottom right pixel (max)
    unsigned area;           ///< Number of pixels in blob
    Eigen::Vector2d mean;    ///< Mean
//     Eigen::Matrix2d covar;   ///< Covarience

    std::set<Run> runs;      ///< Runs in this blob

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    inline friend std::ostream& operator<<(std::ostream& stream, Blob const& blob)
    {
      return stream << "Blob (ul=[" << blob.ul.transpose() << "] br=[" << blob.br.transpose() << "])";
    }
  };
}

EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(bold::Blob)

namespace bold
{
  class SequentialTimer;

  /** Blob detection image pass
   *
   * Builds blobs while passing through an image
   **/
  class BlobDetectPass : public ImagePassHandler
  {
  public:
    BlobDetectPass(ushort imageWidth, ushort imageHeight, std::vector<std::shared_ptr<PixelLabel>> const& blobTypes);

    void clear();

    void process(ImageLabelData const& labelData, SequentialTimer& timer) override;

    std::vector<std::shared_ptr<PixelLabel>> pixelLabels() const { return d_pixelLabels; }

    // Processes Runs into Blobs. Returns a set of blobs per label
    std::map<std::shared_ptr<PixelLabel>,std::vector<Blob>> const& detectBlobs(SequentialTimer& timer);

    std::map<std::shared_ptr<PixelLabel>,std::vector<Blob>> const& getDetectedBlobs() const { return d_blobsDetectedPerLabel; }

    static Blob runSetToBlob(std::set<Run> const& runSet);

  private:
    typedef std::vector<std::vector<Run>> RunLengthCode;

    void addRun(unsigned endX);

    ushort d_imageHeight;
    ushort d_imageWidth;

    std::vector<std::shared_ptr<PixelLabel>> d_pixelLabels;
    std::vector<unsigned> d_rowIndices;

    // Image pass state Accumulated data for the most recently passed image.
    std::map<uint8_t, RunLengthCode> d_runsPerRowPerLabel;
    Run d_currentRun;
    uint8_t d_currentLabel;

    // Blobs detected
    std::map<std::shared_ptr<PixelLabel>,std::vector<Blob>> d_blobsDetectedPerLabel;
  };


  //// Inline members

  //
  //// -------- BlobDetectPass --------
  //
  inline void BlobDetectPass::process(ImageLabelData const& labelData, SequentialTimer& timer)
  {
    // Clear all persistent data
    for (auto& pair : d_runsPerRowPerLabel)
      for (std::vector<Run>& runs : pair.second)
        runs.clear();

    // TODO size of this vector is known at this point -- avoid reallocation
    d_rowIndices.clear();

    ASSERT(labelData.getLabelledRowCount() > 1);

    timer.timeEvent("Clear");

    for (auto const& row : labelData)
    {
      // TODO VISION might miss last run on last row with this approach -- add
      // onRowEnding, or copy into onImageComplete
      if (d_currentLabel != 0)
      {
        // finish whatever run we were on
        addRun(d_imageWidth - 1u);
      }
      d_currentRun.y = row.imageY;
      d_currentLabel = 0;
      d_rowIndices.push_back(row.imageY);

      ushort x = 0;
      for (auto const& label : row)
      {
        // Check if we have a run boundary
        if (label != d_currentLabel)
        {
          // Check whether this is the end of the current run
          if (d_currentLabel != 0)
          {
            // Finished run
            addRun(x - 1u);
          }

          // Check whether this is the start of a new run
          if (label != 0)
          {
            // Start new run
            d_currentRun.startX = x;
          }

          d_currentLabel = label;
        }
        x += row.granularity.x();
      }
    }
    timer.timeEvent("Process Rows");
  }

  inline void BlobDetectPass::clear()
  {
    // Clear all persistent data
    for (auto& pair : d_runsPerRowPerLabel)
      for (std::vector<Run>& runs : pair.second)
        runs.clear();
    d_rowIndices.clear();
    for (auto& pair : d_blobsDetectedPerLabel)
    {
      auto& blobs = pair.second;
      blobs.clear();
    }
  }

  inline void BlobDetectPass::addRun(unsigned endX)
  {
    // finish whatever run we were on
    d_currentRun.endX = endX;

    // TODO do this with pointer arithmetic rather than a map lookup
    auto it = d_runsPerRowPerLabel.find(d_currentLabel);
    if (it != d_runsPerRowPerLabel.end())
      it->second[d_currentRun.y].push_back(d_currentRun);
  }

  //
  //// -------- Run --------
  //
  inline Run::Run(unsigned startX, unsigned y)
    : y(y),
      startX(startX),
      endX(startX)
  {}

  inline Run::Run(unsigned startX, unsigned endX, unsigned y)
    : y(y),
      startX(startX),
      endX(endX)
  {}

  inline unsigned Run::length() const
  {
    return endX - startX + 1;
  }

  inline bool Run::overlaps(Run const& b) const
  {
    return std::max(endX, b.endX) - std::min(startX, b.startX) < length() + b.length();
  }

  inline bool Run::operator<(Run const& other) const
  {
    return
      y < other.y ||
      (y == other.y && startX < other.startX);
  }

  //
  //// -------- Blob --------
  //
  inline Blob::Blob()
    : ul(1e6,1e6),
      br(-1,-1),
      area(0),
      mean(Eigen::Vector2d::Zero())
//       covar(Eigen::Matrix2d::Zero()
  {}

  inline Blob::Blob(Eigen::Vector2i const& _ul, Eigen::Vector2i const& _br,
                    unsigned _area,
                    Eigen::Vector2d _mean, //Eigen::Matrix2d _covar,
                    std::set<Run> const& _runs)
    : ul(_ul),
      br(_br),
      area(_area),
      mean(_mean),
//       covar(_covar),
      runs(_runs)
  {}

  inline cv::Rect Blob::toRect() const
  {
    auto size = br - ul;
    return cv::Rect(ul.x(), ul.y(), size.x(), size.y());
  }

  inline Bounds2i Blob::bounds() const
  {
    return Bounds2i(ul, br);
  }

  inline void Blob::merge(Blob& other)
  {
    ASSERT(other.area != 0);
    mean = ((mean * area) + (other.mean * other.area)) / (area + other.area);
    area += other.area;
    // TODO can we do this more nicely using Eigen?
    if (other.ul.x() < ul.x())
      ul.x() = other.ul.x();
    if (other.ul.y() < ul.y())
      ul.y() = other.ul.y();
    if (other.br.x() > br.x())
      br.x() = other.br.x();
    if (other.br.y() > br.y())
      br.y() = other.br.y();
  }

  inline bool Blob::operator<(Blob const& other) const
  {
    return
      area < other.area ||
      (area == other.area && mean.y() < other.mean.y());
  }
}
