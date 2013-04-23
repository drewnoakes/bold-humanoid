#include "gtest/gtest.h"

#include "helpers.hh"

#include "../AgentPosition/agentposition.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

TEST (AgentPositionTests, worldToAgentTransform)
{
  //
  // Coincident axes
  //

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(1,2,3),
    AgentPosition(0,0, 0).agentWorldTransform() * Vector3d(1,2,3)
  ));

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(3,2,1),
    AgentPosition(0,0, 0).agentWorldTransform() * Vector3d(3,2,1)
  ));

  //
  // Translation
  //

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(0,0,3),
    AgentPosition(1,2, 0).agentWorldTransform() * Vector3d(1,2,3)
  ));

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(2,0,1),
    AgentPosition(1,2, 0).agentWorldTransform() * Vector3d(3,2,1)
  ));

  //
  // Rotation
  //

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(1,0,0),
    AgentPosition(0,0, M_PI/2).agentWorldTransform() * Vector3d(0,1,0)
  ));

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(0,-1,0),
    AgentPosition(0,0, M_PI/2).agentWorldTransform() * Vector3d(1,0,0)
  ));

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(0,1,0),
    AgentPosition(0,0, -M_PI/2).agentWorldTransform() * Vector3d(1,0,0)
  ));

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(0,0,1),
    AgentPosition(0,0, -M_PI/2).agentWorldTransform() * Vector3d(0,0,1)
  ));

  //
  // Rotation & translation
  //

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(0,0,0),
    AgentPosition(1,0, M_PI/2).agentWorldTransform() * Vector3d(1,0,0)
  ));

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(-2,0,0),
    AgentPosition(-2,0, M_PI).agentWorldTransform() * Vector3d(0,0,0) // 90 degrees turned right, facing down -y on the field
  ));
}

TEST (AgentPositionTests, agentToWorldTransform)
{
  //
  // Coincident axes
  //

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(1,2,3),
    AgentPosition(0,0, 0).worldAgentTransform() * Vector3d(1,2,3)
  ));

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(3,2,1),
    AgentPosition(0,0, 0).worldAgentTransform() * Vector3d(3,2,1)
  ));

  //
  // Translation
  //

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(2,4,3),
    AgentPosition(1,2, 0).worldAgentTransform() * Vector3d(1,2,3)
  ));

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(4,4,1),
    AgentPosition(1,2, 0).worldAgentTransform() * Vector3d(3,2,1)
  ));

  //
  // Rotation
  //

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(-1,0,0),
    AgentPosition(0,0, M_PI/2).worldAgentTransform() * Vector3d(0,1,0)
  ));

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(0,1,0),
    AgentPosition(0,0, M_PI/2).worldAgentTransform() * Vector3d(1,0,0)
  ));

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(0,-1,0),
    AgentPosition(0,0, -M_PI/2).worldAgentTransform() * Vector3d(1,0,0)
  ));

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(0,0,1),
    AgentPosition(0,0, -M_PI/2).worldAgentTransform() * Vector3d(0,0,1)
  ));

  //
  // Rotation & translation
  //

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(1,1,0),
    AgentPosition(1,0, M_PI/2).worldAgentTransform() * Vector3d(1,0,0)
  ));

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(-2,0,0),
    AgentPosition(-2,0, M_PI).worldAgentTransform() * Vector3d(0,0,0) // 90 degrees turned right, facing down -y on the field
  ));
}
