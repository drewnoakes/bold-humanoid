#ifndef BOLD_BLOBDETECTPASS_HH
#define BOLD_BLOBDETECTPASS_HH

#include <opencv2/core/core.hpp>
#include <Eigen/Core>

#include <algorithm>
#include <cassert>
#include <memory>
#include <set>
#include <vector>

#include "../imagepasshandler.hh"
#include "../../DisjointSet/disjointset.hh"
#include "../../PixelLabel/pixellabel.hh"

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

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

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
         Eigen::Vector2f _mean, Eigen::Matrix2f _covar,
         std::set<Run> const& _runs);

    cv::Rect toRect() const;

    bool operator<(Blob const& other) const;
    bool operator>(Blob const& other) const { return other < *this; }

    Eigen::Vector2i ul;      ///< Upper left pixel
    Eigen::Vector2i br;      ///< Bottom righ pixel
    unsigned area;           ///< Number of pixes in blob
    Eigen::Vector2f mean;    ///< Mean
    Eigen::Matrix2f covar;   ///< Covarience

    std::set<Run> runs;      ///< Runs in this blob

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    inline friend std::ostream& operator<<(std::ostream& stream, Blob const& blob)
    {
      return stream << "Blob (ul=[" << blob.ul.transpose() << "] br=[" << blob.br.transpose() << "])";
    }
  };

  /** Blob detection image pass
   *
   * Builds blobs while passing through an image
   **/
  class BlobDetectPass : public ImagePassHandler<uchar>
  {
  public:
    BlobDetectPass(int imageWidth, int imageHeight, std::vector<std::shared_ptr<PixelLabel>> const& blobTypes);

    void onImageStarting() override;

    void onRowStarting(int y) override;

    void onPixel(uchar label, int x, int y) override;

    std::vector<std::shared_ptr<PixelLabel>> pixelLabels() const { return d_pixelLabels; }

    // Processes Runs into Blobs. Returns a set of blobs per label
    std::map<std::shared_ptr<PixelLabel>,std::vector<Blob>> const& detectBlobs();

    std::map<std::shared_ptr<PixelLabel>,std::vector<Blob>> const& getDetectedBlobs() const { return d_blobsDetectedPerLabel; }

    static Blob runSetToBlob(std::set<Run> const& runSet);

  private:
    typedef std::vector<std::vector<Run>> RunLengthCode;

    int d_imageHeight;
    int d_imageWidth;

    std::vector<std::shared_ptr<PixelLabel>> d_pixelLabels;

    // Image pass state Accumulated data for the most recently passed image.
    std::map<uchar, RunLengthCode> d_runsPerRowPerLabel;
    Run d_currentRun;
    uchar d_currentLabel;

    // Blobs detected
    std::map<std::shared_ptr<PixelLabel>,std::vector<Blob>> d_blobsDetectedPerLabel;

    void addRun(unsigned endX);
  };


  //// Inline members

  //
  //// -------- BlobDetectPass --------
  //
  inline void BlobDetectPass::onImageStarting()
  {
    // Clear all persistent data
    for (auto& pair : d_runsPerRowPerLabel)
      for (std::vector<Run>& runs : pair.second)
        runs.clear();
  }

  inline void BlobDetectPass::onRowStarting(int y)
  {
    // TODO might miss last run on last row with this approach -- add
    // onRowEnding, or copy into onImageComplete
    if (d_currentLabel != 0)
    {
      // finish whatever run we were on
      addRun(d_imageWidth - 1);
    }
    d_currentRun.y = y;
    d_currentLabel = 0;
  }

  inline void BlobDetectPass::onPixel(uchar label, int x, int y)
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

  inline void BlobDetectPass::addRun(unsigned endX)
  {
    assert(endX >= d_currentRun.startX);

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
      mean(Eigen::Vector2f::Zero()),
      covar(Eigen::Matrix2f::Zero())
  {
  }

  inline Blob::Blob(Eigen::Vector2i const& _ul, Eigen::Vector2i const& _br,
                    unsigned _area,
                    Eigen::Vector2f _mean, Eigen::Matrix2f _covar,
                    std::set<Run> const& _runs)
    : ul(_ul),
      br(_br),
      area(_area),
      mean(_mean),
      covar(_covar),
      runs(_runs)
  {}

  inline cv::Rect Blob::toRect() const
  {
    auto size = br - ul;
    return cv::Rect(ul.x(), ul.y(), size.x(), size.y());
  }

  inline bool Blob::operator<(Blob const& other) const
  {
    return
      area < other.area ||
      (area == other.area && mean.y() < other.mean.y());
  }

}


#endif
