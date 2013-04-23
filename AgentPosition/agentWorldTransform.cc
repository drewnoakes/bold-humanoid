#include "agentposition.hh"

using namespace bold;
using namespace Eigen;

Affine3d AgentPosition::agentWorldTransform() const
{
  return AngleAxisd(-theta(), Vector3d::UnitZ()) * Translation3d(-pos3d());
}
