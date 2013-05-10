#pragma once

#include <vector>
#include <Eigen/Core>

#include "../geometry/LineSegment.hh"

class minIni;

namespace bold
{
  class FieldMap
  {
  public:
    FieldMap(minIni const& ini);

    /// Returns the positions of all field lines, in the world frame.
    std::vector<LineSegment3d> getFieldLines() const { return d_fieldLines; }

    std::vector<LineSegment3d> getCircleLines() const { return d_circleLines; }

    /// Positions of the base of four goal posts, in the world frame.
    std::vector<Eigen::Vector3d> getGoalPostPositions() const { return d_goalPostPositions; }

    /// The long length of the field, from goal to goal.
    double fieldLengthX() const { return d_fieldLengthX; }

    /// The narrow length of the field, from sideline to sideline.
    double fieldLengthY() const { return d_fieldLengthY; }

    /// The minimum width of field surface found outside the outer field lines.
    double outerMarginMinimum() const { return d_outerMarginMinimum; }

    double circleRadius() const { return d_circleRadius; }

  private:
    std::vector<LineSegment3d> d_fieldLines;
    std::vector<LineSegment3d> d_circleLines;
    std::vector<Eigen::Vector3d> d_goalPostPositions;

    double d_fieldLengthX;
    double d_fieldLengthY;
    double d_outerMarginMinimum;
    double d_circleRadius;
  };
}
