#include "agentposition.hh"

using namespace bold;
using namespace Eigen;

Affine3d AgentPosition::worldAgentTransform() const
{
  return Translation3d(pos3d()) * AngleAxisd(theta(), Vector3d::UnitZ());
}
