#include "agentposition.hh"

using namespace bold;
using namespace Eigen;

Affine3d AgentPosition::worldToAgentTransform() const
{
  return AngleAxisd(-theta(), Vector3d::UnitZ()) * Translation3d(-pos());
}
