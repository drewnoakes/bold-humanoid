#pragma once

#include <vector>
#include <Eigen/Core>

#include "../geometry/LineSegment/linesegment.hh"

namespace bold
{
  enum class FieldSide
  {
    Unknown,
    Ours,
    Theirs
  };

  class FieldMap
  {
  public:
    static void initialise();

    // TODO return vectors of const objects so no user can modify them

    /// Returns the positions of all field lines, in the world frame.
    static std::vector<LineSegment3d> const& getFieldLines() { return d_fieldLines; }

    /// Returns the positions of all field line edges (two per field line), in the world frame.
    static std::vector<LineSegment3d> const& getFieldLineEdges() { return d_fieldLineEdges; }

    static std::vector<LineSegment3d> const& getCircleLines() { return d_circleLines; }

    /// Positions of the base of four goal posts, in the world frame.
    static std::vector<Eigen::Vector3d> const& getGoalPostPositions() { return d_goalPostPositions; }

    /// Positions of the base of our two goal posts (which we defend), in the world frame.
    static std::vector<Eigen::Vector3d> const& getOurGoalPostPositions() { return d_ourGoalPostPositions; }

    /// Positions of the base of our their goal posts (which we attack), in the world frame.
    static std::vector<Eigen::Vector3d> const& getTheirGoalPostPositions() { return d_theirGoalPostPositions; }

    static double getMaxDiagonalFieldDistance() { return d_maxDiagnoalFieldDistance; }

    /// The long length of the field, from goal to goal.
    static double fieldLengthX() { return d_fieldLengthX; }

    /// The narrow length of the field, from sideline to sideline.
    static double fieldLengthY() { return d_fieldLengthY; }

    /// The minimum width of field surface found outside the outer field lines.
    static double outerMarginMinimum() { return d_outerMarginMinimum; }

    static double circleRadius() { return d_circleRadius; }
    /// The distance between the centre of goal posts along the y axis.
    static double getGoalY() { return d_goalY; };

  private:
    FieldMap() = delete;

    static std::vector<LineSegment3d> d_fieldLines;
    static std::vector<LineSegment3d> d_fieldLineEdges;
    static std::vector<LineSegment3d> d_circleLines;
    static std::vector<Eigen::Vector3d> d_goalPostPositions;
    static std::vector<Eigen::Vector3d> d_ourGoalPostPositions;
    static std::vector<Eigen::Vector3d> d_theirGoalPostPositions;

    static double d_fieldLengthX;
    static double d_fieldLengthY;
    static double d_outerMarginMinimum;
    static double d_circleRadius;
    static double d_maxDiagnoalFieldDistance;
    static double d_goalY;
  };
}
