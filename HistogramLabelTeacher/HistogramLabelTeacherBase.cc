#include "histogramlabelteacher.ih"

HistogramLabelTeacherBase::HistogramLabelTeacherBase()
  : d_trainImage{},
  d_seedPoint{0,0},
  d_maxDiff{0},
  d_snapshotRequested(false)
{
  Config::addAction("histogramlabelteacher.snap-train-image", "Snap Image", [this]()
                    {
                      d_snapshotRequested = true;
                    });
}
