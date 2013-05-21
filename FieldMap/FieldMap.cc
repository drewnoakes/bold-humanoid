#include "fieldmap.hh"

#include "../geometry/LineSegment.hh"
#include "../Configurable/configurable.hh"

#include <iostream>
#include <vector>
#include <Eigen/Core>

using namespace bold;
using namespace std;
using namespace Eigen;

FieldMap::FieldMap()
  : Configurable("field")
{
  std::cout << "[FieldMap::FieldMap] Start" << std::endl;

  d_fieldLengthX              = getParam("FieldSizeX", 6.0);
  d_fieldLengthY              = getParam("FieldSizeY", 4.0);
//   double goalX               = getParam("GoalSizeX", 0.5);
  double goalY               = getParam("GoalSizeY", 1.5);
//   double goalZ               = getParam("GoalSizeZ", 0.8);
//   double goalPostDiameter    = getParam("GoalPostDiameter", 0.1);
  double goalAreaX           = getParam("GoalAreaSizeX", 0.6);
  double goalAreaY           = getParam("GoalAreaSizeY", 2.2);
//   double penaltyMarkDistance = getParam("PenaltyMarkDistance", 1.8);
  double circleDiameter      = getParam("CircleDiameter", 1.2);
//   double lineWidth           = getParam("LineWidth", 0.05);
//   double penaltyLineLength   = getParam("PenaltyLineLength", 0.1);
  d_outerMarginMinimum  = getParam("OuterMarginMinimum", 0.7);
//   double ballDiameter        = getParam("BallDiameter", 0.067); // according to Wikipedia

//   double halfCrossLength = penaltyLineLength/2;
//   double penaltyX = d_fieldLengthX/2 - penaltyMarkDistance;
//   double penaltyInnerX = penaltyX - halfCrossLength;
//   double penaltyOuterX = penaltyX + halfCrossLength;
  double halfFieldX = d_fieldLengthX/2;
  double halfFieldY = d_fieldLengthY/2;
  double halfGoalAreaY = goalAreaY/2;
  double halfGoalY = goalY/2;
  d_circleRadius = circleDiameter/2;

/*
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
*/

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

  // CIRCLE
  int segments = 18;
  Vector3d lastPoint(0, d_circleRadius, 0);
  for (unsigned i = 1; i <= segments; i++)
  {
    double theta = (i/(double)segments) * M_PI * 2;
    Vector3d point(sin(theta) * d_circleRadius, cos(theta) * d_circleRadius, 0);
    d_circleLines.push_back(LineSegment3d(lastPoint, point));
    lastPoint = point;
  }

  // GOAL POST POSITIONS
  d_goalPostPositions = {
    Vector3d(-halfFieldX, halfGoalY, 0),
    Vector3d(-halfFieldX, -halfGoalY, 0),
    Vector3d(halfFieldX, halfGoalY, 0),
    Vector3d(halfFieldX, -halfGoalY, 0)
  };
};
