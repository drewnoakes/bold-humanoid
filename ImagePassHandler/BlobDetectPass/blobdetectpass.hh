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

typedef unsigned short ushort;
typedef Eigen::Matrix<ushort, 2, 1> ImagePos;
typedef bold::Bounds2<ushort> ImageBounds;

namespace bold
{
  /** Horizontal run of pixels
   *
   **/
  struct Run
  {
    Run(ushort startX, ushort y);
    Run(ushort startX, ushort endX, ushort y);

    /** Returns the number of pixels in the run.
     *
     * If a run starts and stops at the same x position, it has length one.
     **/
    ushort length() const;

    /** Returns whether two runs overlap, ignoring y positions. */
    bool overlaps(Run const& b) const;

    /** Compare operator
     *
     * Orders runs by y, then by startX.
     */
    bool operator<(Run const& other) const;

    ushort y;        ///< The row index of the horizontal run
    ushort startX;   ///< The column index of the left-most pixel
    ushort endX;     ///< The column index of the right-most pixel

    float midX() const { return (endX + startX) / 2.0f; }

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
    Blob(ImagePos const& _ul, ImagePos const& _br,
         unsigned _area,
         Eigen::Vector2f _mean, // Eigen::Matrix2f _covar,
         std::set<Run> const& _runs);

    cv::Rect toRect() const;

    ImageBounds bounds() const;

    void merge(Blob& other);

    bool operator<(Blob const& other) const;
    bool operator>(Blob const& other) const { return other < *this; }

    ImagePos ul;             ///< Upper left pixel (min)
    ImagePos br;             ///< Bottom right pixel (max)
    unsigned area;           ///< Number of pixels in blob
    Eigen::Vector2f mean;    ///< Mean
//  Eigen::Matrix2f covar;   ///< Covarience

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

    void addRun(Run& run, uint8_t label);

    ushort d_imageHeight;
    ushort d_imageWidth;

    std::vector<std::shared_ptr<PixelLabel>> d_pixelLabels;
    std::vector<ushort> d_rowIndices;

    // Image pass state Accumulated data for the most recently passed image.
    std::map<uint8_t, RunLengthCode> d_runsPerRowPerLabel;

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

    uint8_t currentLabel = 0;
    Run currentRun(0, 0);

    ASSERT(labelData.getLabelledRowCount() > 1);

    timer.timeEvent("Clear");

    for (auto const& row : labelData)
    {
      // TODO VISION might miss last run on last row with this approach
      if (currentLabel != 0)
      {
        // finish whatever run we were on
        currentRun.endX = d_imageWidth - (ushort)1;
        addRun(currentRun, currentLabel);
      }
      currentRun.y = row.imageY;
      currentLabel = 0;
      d_rowIndices.push_back(row.imageY);

      ushort x = 0;
      for (auto const& label : row)
      {
        // Check if we have a run boundary
        if (label != currentLabel)
        {
          // Check whether this is the end of the current run
          if (currentLabel != 0)
          {
            // Finished run
            currentRun.endX = x - (ushort)1;
            addRun(currentRun, currentLabel);
          }

          // Check whether this is the start of a new run
          if (label != 0)
          {
            // Start new run
            currentRun.startX = x;
          }

          currentLabel = label;
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

  inline void BlobDetectPass::addRun(Run& run, uint8_t label)
  {
    // TODO PERFORMANCE do this with pointer arithmetic rather than a map lookup
    auto it = d_runsPerRowPerLabel.find(label);
    if (it != d_runsPerRowPerLabel.end())
      it->second[run.y].push_back(run);
  }

  //
  //// -------- Run --------
  //
  inline Run::Run(ushort startX, ushort y)
    : y(y),
      startX(startX),
      endX(startX)
  {}

  inline Run::Run(ushort startX, ushort endX, ushort y)
    : y(y),
      startX(startX),
      endX(endX)
  {}

  inline ushort Run::length() const
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
    : ul(std::numeric_limits<ushort>::max(),std::numeric_limits<ushort>::max()),
      br(std::numeric_limits<ushort>::min(),std::numeric_limits<ushort>::min()),
      area(0),
      mean(Eigen::Vector2f::Zero())
//    covar(Eigen::Matrix2f::Zero()
  {}

  inline Blob::Blob(ImagePos const& _ul, ImagePos const& _br,
                    unsigned _area,
                    Eigen::Vector2f _mean, //Eigen::Matrix2f _covar,
                    std::set<Run> const& _runs)
    : ul(_ul),
      br(_br),
      area(_area),
      mean(_mean),
//    covar(_covar),
      runs(_runs)
  {}

  inline cv::Rect Blob::toRect() const
  {
    auto size = br - ul;
    return cv::Rect(ul.x(), ul.y(), size.x(), size.y());
  }

  inline ImageBounds Blob::bounds() const
  {
    return ImageBounds(ul, br);
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
