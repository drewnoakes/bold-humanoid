#include "fieldmap.hh"

#include "../geometry/LineSegment.hh"

#include <minIni.h>
#include <iostream>
#include <vector>
#include <Eigen/Core>

using namespace bold;
using namespace std;
using namespace Eigen;

FieldMap::FieldMap(minIni const& ini)
{
  std::cout << "[FieldMap::FieldMap] Start" << std::endl;

  d_fieldLengthX              = ini.getd("Environment", "FieldSizeX", 6.0);
  d_fieldLengthY              = ini.getd("Environment", "FieldSizeY", 4.0);
//   double goalX               = ini.getd("Environment", "GoalSizeX", 0.5);
  double goalY               = ini.getd("Environment", "GoalSizeY", 1.5);
//   double goalZ               = ini.getd("Environment", "GoalSizeZ", 0.8);
//   double goalPostDiameter    = ini.getd("Environment", "GoalPostDiameter", 0.1);
  double goalAreaX           = ini.getd("Environment", "GoalAreaSizeX", 0.6);
  double goalAreaY           = ini.getd("Environment", "GoalAreaSizeY", 2.2);
//   double penaltyMarkDistance = ini.getd("Environment", "PenaltyMarkDistance", 1.8);
  double circleDiameter      = ini.getd("Environment", "CircleDiameter", 1.2);
//   double lineWidth           = ini.getd("Environment", "LineWidth", 0.05);
//   double penaltyLineLength   = ini.getd("Environment", "PenaltyLineLength", 0.1);
  d_outerMarginMinimum  = ini.getd("Environment", "OuterMarginMinimum", 0.7);
//   double ballDiameter        = ini.getd("Environment", "BallDiameter", 0.067); // according to Wikipedia

//   double halfCrossLength = penaltyLineLength/2;
//   double penaltyX = fieldX/2 - penaltyMarkDistance;
//   double penaltyInnerX = penaltyX - halfCrossLength;
//   double penaltyOuterX = penaltyX + halfCrossLength;
  double halfFieldX = d_fieldLengthX/2;
  double halfFieldY = d_fieldLengthY/2;
  double halfGoalAreaY = goalAreaY/2;
  double halfGoalY = goalY/2;
  double circleRadius = circleDiameter/2;

  // TODO store the three cross marks

//   // CROSS MARKS
//   // center cross mark
//   LineSegment2d(-halfCrossLength, 0, halfCrossLength, 0);
//   LineSegment2d(0, -halfCrossLength, 0, halfCrossLength);
//   // left penalty mark
//   LineSegment2d(-penaltyInnerX, 0, -penaltyOuterX, 0);
//   LineSegment2d(-penaltyX, -halfCrossLength, -penaltyX, halfCrossLength);
//   // right penalty mark
//   LineSegment2d(penaltyInnerX, 0, penaltyOuterX, 0);
//   LineSegment2d(penaltyX, -halfCrossLength, penaltyX, halfCrossLength);

  // OUTER SQUARE
  // top
  d_fieldLines.push_back(LineSegment2d(Vector2d(-halfFieldX, halfFieldY), Vector2d(halfFieldX, halfFieldY)));
  // bottom
  d_fieldLines.push_back(LineSegment2d(Vector2d(-halfFieldX, -halfFieldY), Vector2d(halfFieldX, -halfFieldY)));
  // left
  d_fieldLines.push_back(LineSegment2d(Vector2d(-halfFieldX, -halfFieldY), Vector2d(-halfFieldX, halfFieldY)));
  // right
  d_fieldLines.push_back(LineSegment2d(Vector2d(halfFieldX, -halfFieldY), Vector2d(halfFieldX, halfFieldY)));

  // GOAL AREAS
  // left, top
  d_fieldLines.push_back(LineSegment2d(Vector2d(-halfFieldX, halfGoalAreaY), Vector2d(-halfFieldX+goalAreaX, halfGoalAreaY)));
  // left, bottom
  d_fieldLines.push_back(LineSegment2d(Vector2d(-halfFieldX, -halfGoalAreaY), Vector2d(-halfFieldX+goalAreaX, -halfGoalAreaY)));
  // left, side
  d_fieldLines.push_back(LineSegment2d(Vector2d(-halfFieldX+goalAreaX, halfGoalAreaY), Vector2d(-halfFieldX+goalAreaX, -halfGoalAreaY)));
  // right, top
  d_fieldLines.push_back(LineSegment2d(Vector2d(halfFieldX, halfGoalAreaY), Vector2d(halfFieldX-goalAreaX, halfGoalAreaY)));
  // right, bottom
  d_fieldLines.push_back(LineSegment2d(Vector2d(halfFieldX, -halfGoalAreaY), Vector2d(halfFieldX-goalAreaX, -halfGoalAreaY)));
  // right, side
  d_fieldLines.push_back(LineSegment2d(Vector2d(halfFieldX-goalAreaX, halfGoalAreaY), Vector2d(halfFieldX-goalAreaX, -halfGoalAreaY)));

  // CIRCLE
  int segments = 18;
  Vector2d lastPoint(0, circleRadius);
  for (unsigned i = 1; i <= segments; i++)
  {
    double theta = (i/(double)segments) * M_PI * 2;
    Vector2d point(sin(theta) * circleRadius, cos(theta) * circleRadius);
    d_fieldLines.push_back(LineSegment2d(lastPoint, point));
    lastPoint = point;
  }

  // GOAL POST POSITIONS
  d_goalPostPositions = {
    Vector2d(-halfFieldX, halfGoalY),
    Vector2d(-halfFieldX, -halfGoalY),
    Vector2d(halfFieldX, halfGoalY),
    Vector2d(halfFieldX, -halfGoalY)
  };
};