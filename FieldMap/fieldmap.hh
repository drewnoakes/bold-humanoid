#ifndef BOLD_WORLD_MODEL_HH
#define BOLD_WORLD_MODEL_HH

#include <minIni.h>
#include <vector>
#include <Eigen/Core>

#include "../geometry/LineSegment.hh"

namespace bold
{
  class FieldMap
  {
  public:
    FieldMap(minIni const& ini);

    std::vector<LineSegment2d> getFieldLines() const { return d_fieldLines; }

    /** Positions of the base of four goal posts. */
    std::vector<Eigen::Vector2d> getGoalPostPositions() const { return d_goalPostPositions; }

    double fieldLengthX() const { return d_fieldLengthX; }
    double fieldLengthY() const { return d_fieldLengthY; }
    double outerMarginMinimum() const { return d_outerMarginMinimum; }

  private:
    std::vector<LineSegment2d> d_fieldLines;
    std::vector<Eigen::Vector2d> d_goalPostPositions;

    double d_fieldLengthX;
    double d_fieldLengthY;
    double d_outerMarginMinimum;
  };
}

#endif
