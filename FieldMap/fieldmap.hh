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

    /// Returns the positions of all field lines, in the world frame.
    std::vector<LineSegment2d> getFieldLines() const { return d_fieldLines; }

    /// Positions of the base of four goal posts, in the world frame.
    std::vector<Eigen::Vector2d> getGoalPostPositions() const { return d_goalPostPositions; }

    /// The long length of the field, from goal to goal.
    double fieldLengthX() const { return d_fieldLengthX; }

    /// The narrow length of the field, from sideline to sideline.
    double fieldLengthY() const { return d_fieldLengthY; }

    /// The minimum width of field surface found outside the outer field lines.
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
