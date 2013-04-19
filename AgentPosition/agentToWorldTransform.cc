#include "agentposition.hh"

using namespace bold;
using namespace Eigen;

Affine3d AgentPosition::agentToWorldTransform() const
{
  return Translation3d(pos()) * AngleAxisd(theta(), Vector3d::UnitZ());
}
