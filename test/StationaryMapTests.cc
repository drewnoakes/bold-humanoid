#include "gtest/gtest.h"

#include <Eigen/Core>

#include "helpers.hh"
#include "../Config/config.hh"
#include "../FieldMap/fieldmap.hh"
#include "../StateObject/StationaryMapState/stationarymapstate.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

TEST (GoalEstimateTests, estimateOppositeGoal)
{
  GoalEstimate goal(Vector2d(-1, 0), Vector2d(-1, 1), GoalLabel::Unknown);

  GoalEstimate opposite(goal.estimateOppositeGoal(GoalLabel::Ours));

  EXPECT_EQ ( Vector2d(FieldMap::getFieldLengthX() - 1, 0), opposite.getPost1Pos() );
  EXPECT_EQ ( Vector2d(FieldMap::getFieldLengthX() - 1, 1), opposite.getPost2Pos() );
  EXPECT_EQ ( GoalLabel::Ours, opposite.getLabel() );

  goal = { Vector2d(1, 0), Vector2d(1, 1), GoalLabel::Unknown };

  opposite = goal.estimateOppositeGoal(GoalLabel::Theirs);

  EXPECT_EQ ( Vector2d(-FieldMap::getFieldLengthX() + 1, 0), opposite.getPost1Pos() );
  EXPECT_EQ ( Vector2d(-FieldMap::getFieldLengthX() + 1, 1), opposite.getPost2Pos() );
  EXPECT_EQ ( GoalLabel::Theirs, opposite.getLabel() );

  goal = { Vector2d(0, 1), Vector2d(1, 1), GoalLabel::Unknown };

  opposite = goal.estimateOppositeGoal(GoalLabel::Theirs);

  EXPECT_EQ ( Vector2d(0, -FieldMap::getFieldLengthX() + 1), opposite.getPost1Pos() );
  EXPECT_EQ ( Vector2d(1, -FieldMap::getFieldLengthX() + 1), opposite.getPost2Pos() );
  EXPECT_EQ ( GoalLabel::Theirs, opposite.getLabel() );

  goal = { Vector2d(-1, -1), Vector2d(1, -1), GoalLabel::Unknown };

  opposite = goal.estimateOppositeGoal(GoalLabel::Theirs);

  EXPECT_EQ ( Vector2d(-1, FieldMap::getFieldLengthX() - 1), opposite.getPost1Pos() );
  EXPECT_EQ ( Vector2d( 1, FieldMap::getFieldLengthX() - 1), opposite.getPost2Pos() );
  EXPECT_EQ ( GoalLabel::Theirs, opposite.getLabel() );
}

TEST (GoalEstimateTests, getMidpoint)
{
  GoalEstimate goal(Vector2d(0, 0), Vector2d(1, 0), GoalLabel::Unknown);

  EXPECT_EQ ( Vector2d(0.0, 0), goal.getMidpoint(0)   );
  EXPECT_EQ ( Vector2d(0.1, 0), goal.getMidpoint(0.1) );
  EXPECT_EQ ( Vector2d(0.5, 0), goal.getMidpoint(0.5) );
  EXPECT_EQ ( Vector2d(1.0, 0), goal.getMidpoint(1.0) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
Average<T> createAverage(T value, int count)
{
  Average<T> avg;
  for (; count > 0; count--)
    avg.add(value);
  return avg;
}

TEST (StationaryMapTests, pairGoalPosts)
{
  // TODO give a case where one of the posts could be paired with two of the others, yet one pairing is more probable

  auto goalY = FieldMap::getGoalY();

  vector<Average<Vector2d>> posts = {
    createAverage(Vector2d(0, 0),     StationaryMapState::GoalSamplesNeeded),
    createAverage(Vector2d(0.1, 0.1), StationaryMapState::GoalSamplesNeeded),
    createAverage(Vector2d(0, goalY), StationaryMapState::GoalSamplesNeeded),
    createAverage(Vector2d(100, 100), StationaryMapState::GoalSamplesNeeded)
  };

  auto pairs = StationaryMapState::pairGoalPosts(posts);

  ASSERT_EQ (1, pairs.size());

  EXPECT_TRUE(VectorsEqual(Vector2d(0,0), pairs[0].first.getAverage()));
  EXPECT_TRUE(VectorsEqual(Vector2d(0,goalY), pairs[0].second.getAverage()));


  vector<Average<Vector2d>> uselessPosts = {
    createAverage(Vector2d(0, 0),     StationaryMapState::GoalSamplesNeeded),
    createAverage(Vector2d(0.1, 0.1), StationaryMapState::GoalSamplesNeeded),
    createAverage(Vector2d(100, 100), StationaryMapState::GoalSamplesNeeded)
  };

  ASSERT_EQ (0, StationaryMapState::pairGoalPosts(uselessPosts).size());
}

TEST (StationaryMapTests, labelGoal_usingKeeperBallEstimate)
{
  auto goalY = FieldMap::getGoalY();
  auto fieldX = FieldMap::getFieldLengthX();

  auto post1 = createAverage(Vector2d(1, 0),     StationaryMapState::GoalSamplesNeeded);
  auto post2 = createAverage(Vector2d(1, goalY), StationaryMapState::GoalSamplesNeeded);

  vector<Average<Vector2d>> keepers;

  ASSERT_EQ(GoalLabel::Ours, StationaryMapState::labelGoal(FieldSide::Ours, post1, post2, keepers));
  ASSERT_EQ(GoalLabel::Unknown, StationaryMapState::labelGoal(FieldSide::Unknown, post1, post2, keepers));
  ASSERT_EQ(GoalLabel::Unknown, StationaryMapState::labelGoal(FieldSide::Theirs, post1, post2, keepers));

  post1 = createAverage(Vector2d(1 + fieldX/2.0, 0),     StationaryMapState::GoalSamplesNeeded);
  post2 = createAverage(Vector2d(1 + fieldX/2.0, goalY), StationaryMapState::GoalSamplesNeeded);

  ASSERT_EQ(GoalLabel::Unknown, StationaryMapState::labelGoal(FieldSide::Ours, post1, post2, keepers));

  // TODO if the ball is on our side and the goal is far enough away, then it's Theirs, not Unknown
}

TEST (StationaryMapTests, labelGoal_usingKeeperObservations)
{
  auto goalY = FieldMap::getGoalY();
  auto maxGoalieGoalDistance = Config::getValue<double>("vision.player-detection.max-goalie-goal-dist");

  auto post1 = createAverage(Vector2d(1, 0),     StationaryMapState::GoalSamplesNeeded);
  auto post2 = createAverage(Vector2d(1, goalY), StationaryMapState::GoalSamplesNeeded);

  Vector2d goodKeeper1(1, goalY/2.0);
  Vector2d goodKeeper2(1, goalY/2.0 + 0.99 * maxGoalieGoalDistance);
  Vector2d goodKeeper3(1 + 0.99 * maxGoalieGoalDistance, goalY/2.0);
  Vector2d goodKeeper4(1 - 0.99 * maxGoalieGoalDistance, goalY/2.0);

  Vector2d badKeeper1(1, goalY/2.0 + 1.01 * maxGoalieGoalDistance);
  Vector2d badKeeper2(1, goalY/2.0 - 1.01 * maxGoalieGoalDistance);
  Vector2d badKeeper3(1 + 1.01 * maxGoalieGoalDistance, goalY/2.0);
  Vector2d badKeeper4(1 - 1.01 * maxGoalieGoalDistance, goalY/2.0);

  vector<Average<Vector2d>> keepers = {
    createAverage(badKeeper1, StationaryMapState::KeeperSamplesNeeded),
    createAverage(badKeeper2, StationaryMapState::KeeperSamplesNeeded),
    createAverage(badKeeper3, StationaryMapState::KeeperSamplesNeeded),
    createAverage(badKeeper4, StationaryMapState::KeeperSamplesNeeded)
  };

  // Test with a set of keepers observations that are too far from the midpoint of the goal
  ASSERT_EQ(GoalLabel::Unknown, StationaryMapState::labelGoal(FieldSide::Unknown, post1, post2, keepers));

  keepers[1] = createAverage(goodKeeper1, StationaryMapState::KeeperSamplesNeeded);
  ASSERT_EQ(GoalLabel::Ours, StationaryMapState::labelGoal(FieldSide::Unknown, post1, post2, keepers));

  keepers[1] = createAverage(goodKeeper2, StationaryMapState::KeeperSamplesNeeded);
  ASSERT_EQ(GoalLabel::Ours, StationaryMapState::labelGoal(FieldSide::Unknown, post1, post2, keepers));

  keepers[1] = createAverage(goodKeeper3, StationaryMapState::KeeperSamplesNeeded);
  ASSERT_EQ(GoalLabel::Ours, StationaryMapState::labelGoal(FieldSide::Unknown, post1, post2, keepers));

  keepers[1] = createAverage(goodKeeper4, StationaryMapState::KeeperSamplesNeeded);
  ASSERT_EQ(GoalLabel::Ours, StationaryMapState::labelGoal(FieldSide::Unknown, post1, post2, keepers));
}
