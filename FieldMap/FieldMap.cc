#include "fieldmap.hh"

#include "../Config/config.hh"
#include "../geometry/LineSegment.hh"

#include <vector>
#include <Eigen/Core>
#include <Eigen/Geometry>

using namespace bold;
using namespace std;
using namespace Eigen;

FieldMap::FieldMap()
{
  d_fieldLengthX             = Config::getStaticValue<double>("world.field-size-x");
  d_fieldLengthY             = Config::getStaticValue<double>("world.field-size-y");
//double goalX               = Config::getStaticValue<double>("world.goal-size-x");
  double goalY               = Config::getStaticValue<double>("world.goal-size-y");
//double goalZ               = Config::getStaticValue<double>("world.goal-size-z");
//double goalPostDiameter    = Config::getStaticValue<double>("world.goal-post-diameter");
  double goalAreaX           = Config::getStaticValue<double>("world.goal-area-size-x");
  double goalAreaY           = Config::getStaticValue<double>("world.goal-area-size-y");
  double penaltyMarkDistance = Config::getStaticValue<double>("world.penalty-mark-distance");
  double circleDiameter      = Config::getStaticValue<double>("world.circle-diameter");
  double lineWidth           = Config::getStaticValue<double>("world.line-width");
  double penaltyLineLength   = Config::getStaticValue<double>("world.penalty-line-length");
  d_outerMarginMinimum       = Config::getStaticValue<double>("world.outer-margin-minimum");
//double ballDiameter        = Config::getStaticValue<double>("world.ball-diameter");

  d_maxDiagnoalFieldDistance = Vector2d(
    d_fieldLengthX + 2*d_outerMarginMinimum,
    d_fieldLengthY + 2*d_outerMarginMinimum
  ).norm();

  double halfCrossLength = penaltyLineLength/2;
  double penaltyX = d_fieldLengthX/2 - penaltyMarkDistance;
  double penaltyInnerX = penaltyX - halfCrossLength;
  double penaltyOuterX = penaltyX + halfCrossLength;
  double halfFieldX = d_fieldLengthX/2;
  double halfFieldY = d_fieldLengthY/2;
  double halfGoalAreaY = goalAreaY/2;
  double halfGoalY = goalY/2;
  d_circleRadius = circleDiameter/2;

  // CROSS MARKS
  // center cross mark
  d_fieldLines.push_back(LineSegment3d(Vector3d(-halfCrossLength, 0, 0), Vector3d(halfCrossLength, 0, 0)));
//  d_fieldLines.push_back(LineSegment3d(Vector3d(0, -halfCrossLength, 0), Vector3d(0, halfCrossLength, 0)));
  // left penalty mark
  d_fieldLines.push_back(LineSegment3d(Vector3d(-penaltyInnerX, 0, 0), Vector3d(-penaltyOuterX, 0, 0)));
  d_fieldLines.push_back(LineSegment3d(Vector3d(-penaltyX, -halfCrossLength, 0), Vector3d(-penaltyX, halfCrossLength, 0)));
  // right penalty mark
  d_fieldLines.push_back(LineSegment3d(Vector3d(penaltyInnerX, 0, 0), Vector3d(penaltyOuterX, 0, 0)));
  d_fieldLines.push_back(LineSegment3d(Vector3d(penaltyX, -halfCrossLength, 0), Vector3d(penaltyX, halfCrossLength, 0)));

  // OUTER SQUARE
  // top
  d_fieldLines.push_back(LineSegment3d(Vector3d(-halfFieldX, halfFieldY, 0), Vector3d(halfFieldX, halfFieldY, 0)));
  // bottom
  d_fieldLines.push_back(LineSegment3d(Vector3d(-halfFieldX, -halfFieldY, 0), Vector3d(halfFieldX, -halfFieldY, 0)));
  // left
  d_fieldLines.push_back(LineSegment3d(Vector3d(-halfFieldX, -halfFieldY, 0), Vector3d(-halfFieldX, halfFieldY, 0)));
  // right
  d_fieldLines.push_back(LineSegment3d(Vector3d(halfFieldX, -halfFieldY, 0), Vector3d(halfFieldX, halfFieldY, 0)));

  // GOAL AREAS
  // left, top
  d_fieldLines.push_back(LineSegment3d(Vector3d(-halfFieldX, halfGoalAreaY, 0), Vector3d(-halfFieldX+goalAreaX, halfGoalAreaY, 0)));
  // left, bottom
  d_fieldLines.push_back(LineSegment3d(Vector3d(-halfFieldX, -halfGoalAreaY, 0), Vector3d(-halfFieldX+goalAreaX, -halfGoalAreaY, 0)));
  // left, side
  d_fieldLines.push_back(LineSegment3d(Vector3d(-halfFieldX+goalAreaX, halfGoalAreaY, 0), Vector3d(-halfFieldX+goalAreaX, -halfGoalAreaY, 0)));
  // right, top
  d_fieldLines.push_back(LineSegment3d(Vector3d(halfFieldX, halfGoalAreaY, 0), Vector3d(halfFieldX-goalAreaX, halfGoalAreaY, 0)));
  // right, bottom
  d_fieldLines.push_back(LineSegment3d(Vector3d(halfFieldX, -halfGoalAreaY, 0), Vector3d(halfFieldX-goalAreaX, -halfGoalAreaY, 0)));
  // right, side
  d_fieldLines.push_back(LineSegment3d(Vector3d(halfFieldX-goalAreaX, halfGoalAreaY, 0), Vector3d(halfFieldX-goalAreaX, -halfGoalAreaY, 0)));

  // CENTER LINE
  d_fieldLines.push_back(LineSegment3d(Vector3d(0, -halfFieldY, 0), Vector3d(0, halfFieldY, 0)));

  // CIRCLE
  int segments = Config::getStaticValue<int>("world.circle-segment-count");
  Vector3d lastPoint(0, d_circleRadius, 0);
  for (unsigned i = 1; i <= segments; i++)
  {
    double theta = (i/(double)segments) * M_PI * 2;
    Vector3d point(sin(theta) * d_circleRadius, cos(theta) * d_circleRadius, 0);
    d_circleLines.push_back(LineSegment3d(lastPoint, point));
    d_fieldLines.push_back(LineSegment3d(lastPoint, point));
    lastPoint = point;
  }

  // TODO allow including circle in 'expected lines' view

  for (LineSegment3d const& line : d_fieldLines)
  {
    auto vec = (Vector3d)line.delta();
    auto perp = vec.cross(Vector3d::UnitZ());
    perp.normalize();
    perp *= (lineWidth / 2);

    d_fieldLineEdges.push_back(line + perp);
    d_fieldLineEdges.push_back(line - perp);
  }

  // GOAL POST POSITIONS
  d_goalPostPositions = {
    Vector3d(-halfFieldX, halfGoalY, 0),
    Vector3d(-halfFieldX, -halfGoalY, 0),
    Vector3d(halfFieldX, halfGoalY, 0),
    Vector3d(halfFieldX, -halfGoalY, 0)
  };

  d_ourGoalPostPositions = {
    Vector3d(-halfFieldX, halfGoalY, 0),
    Vector3d(-halfFieldX, -halfGoalY, 0)
  };

  d_theirGoalPostPositions = {
    Vector3d(halfFieldX, halfGoalY, 0),
    Vector3d(halfFieldX, -halfGoalY, 0)
  };
}
