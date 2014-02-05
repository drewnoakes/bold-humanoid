#pragma once

#include <vector>
#include <Eigen/Core>

#include "../geometry/LineSegment.hh"

namespace bold
{
  class FieldMap
  {
  public:
    FieldMap();

    // TODO return vectors of const objects so no user can modify them

    /// Returns the positions of all field lines, in the world frame.
    std::vector<LineSegment3d> const& getFieldLines() const { return d_fieldLines; }

    /// Returns the positions of all field line edges (two per field line), in the world frame.
    std::vector<LineSegment3d> const& getFieldLineEdges() const { return d_fieldLineEdges; }

    std::vector<LineSegment3d> const& getCircleLines() const { return d_circleLines; }

    /// Positions of the base of four goal posts, in the world frame.
    std::vector<Eigen::Vector3d> const& getGoalPostPositions() const { return d_goalPostPositions; }

    /// Positions of the base of our two goal posts (which we defend), in the world frame.
    std::vector<Eigen::Vector3d> const& getOurGoalPostPositions() const { return d_ourGoalPostPositions; }

    /// Positions of the base of our their goal posts (which we attack), in the world frame.
    std::vector<Eigen::Vector3d> const& getTheirGoalPostPositions() const { return d_theirGoalPostPositions; }

    /// The long length of the field, from goal to goal.
    double fieldLengthX() const { return d_fieldLengthX; }

    /// The narrow length of the field, from sideline to sideline.
    double fieldLengthY() const { return d_fieldLengthY; }

    /// The minimum width of field surface found outside the outer field lines.
    double outerMarginMinimum() const { return d_outerMarginMinimum; }

    double circleRadius() const { return d_circleRadius; }

  private:
    std::vector<LineSegment3d> d_fieldLines;
    std::vector<LineSegment3d> d_fieldLineEdges;
    std::vector<LineSegment3d> d_circleLines;
    std::vector<Eigen::Vector3d> d_goalPostPositions;
    std::vector<Eigen::Vector3d> d_ourGoalPostPositions;
    std::vector<Eigen::Vector3d> d_theirGoalPostPositions;

    double d_fieldLengthX;
    double d_fieldLengthY;
    double d_outerMarginMinimum;
    double d_circleRadius;
  };
}
