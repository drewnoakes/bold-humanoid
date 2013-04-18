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
    AgentPosition(0, 0, 0, 0).worldToAgentTransform() * Vector3d(1,2,3)
  ));


  EXPECT_TRUE ( VectorsEqual(
    Vector3d(1,2,3),
    AgentPosition(0, 0, 0, 0).worldToAgentTransform() * Vector3d(1,2,3)
  ));

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(3,2,1),
    AgentPosition(0, 0, 0, 0).worldToAgentTransform() * Vector3d(3,2,1)
  ));

  //
  // Translation
  //

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(0,0,0),
    AgentPosition(1,2,3, 0).worldToAgentTransform() * Vector3d(1,2,3)
  ));

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(2,0,-2),
    AgentPosition(1,2,3, 0).worldToAgentTransform() * Vector3d(3,2,1)
  ));

  //
  // Rotation
  //

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(1,0,0),
    AgentPosition(0,0,0, M_PI/2).worldToAgentTransform() * Vector3d(0,1,0)
  ));

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(0,-1,0),
    AgentPosition(0,0,0, M_PI/2).worldToAgentTransform() * Vector3d(1,0,0)
  ));

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(0,1,0),
    AgentPosition(0,0,0, -M_PI/2).worldToAgentTransform() * Vector3d(1,0,0)
  ));

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(0,0,1),
    AgentPosition(0,0,0, -M_PI/2).worldToAgentTransform() * Vector3d(0,0,1)
  ));

  //
  // Rotation & translation
  //

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(0,0,0),
    AgentPosition(1,0,0, M_PI/2).worldToAgentTransform() * Vector3d(1,0,0)
  ));

  EXPECT_TRUE ( VectorsEqual(
    Vector3d(-2,0,-0.5),
    AgentPosition(-2,0,0.5, M_PI).worldToAgentTransform() * Vector3d(0,0,0) // 90 degrees turned right, facing down -y on the field
  ));
}



