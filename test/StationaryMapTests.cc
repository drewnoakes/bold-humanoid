#include "gtest/gtest.h"

#include <Eigen/Core>

#include "helpers.hh"
#include "../FieldMap/fieldmap.hh"
#include "../StateObject/StationaryMapState/stationarymapstate.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

TEST (GoalEstimateTests, estimateOppositeGoal)
{
  GoalEstimate goal(Vector3d(-1, 0, 0), Vector3d(-1, 1, 0), GoalLabel::Unknown);

  GoalEstimate opposite(goal.estimateOppositeGoal(GoalLabel::Ours));

  EXPECT_EQ ( Vector3d(FieldMap::fieldLengthX() - 1, 0, 0), opposite.getPost1() );
  EXPECT_EQ ( Vector3d(FieldMap::fieldLengthX() - 1, 1, 0), opposite.getPost2() );
  EXPECT_EQ ( GoalLabel::Ours, opposite.getLabel() );
}

TEST (GoalEstimateTests, getMidpoint)
{
  GoalEstimate goal(Vector3d(0, 0, 0), Vector3d(1, 0, 0), GoalLabel::Unknown);

  EXPECT_EQ ( Vector3d(0.0, 0, 0), goal.getMidpoint(0)   );
  EXPECT_EQ ( Vector3d(0.1, 0, 0), goal.getMidpoint(0.1) );
  EXPECT_EQ ( Vector3d(0.5, 0, 0), goal.getMidpoint(0.5) );
  EXPECT_EQ ( Vector3d(1.0, 0, 0), goal.getMidpoint(1.0) );
}
