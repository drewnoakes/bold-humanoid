#include "localiser.ih"

void Localiser::predict()
{
  auto orientationState = State::get<OrientationState>(StateTime::CameraImage);
  auto odometryState = State::get<OdometryState>(StateTime::CameraImage);
  if (!orientationState || !odometryState)
    return;

  if (d_lastQuaternion.norm() > 0)
  {
    auto deltaQuaternion = orientationState->getQuaternion() * d_lastQuaternion.inverse();
    auto transformMatrix = deltaQuaternion.matrix();
    Vector2d forward = transformMatrix.col(1).head<2>();
    double deltaTheta = -atan2(forward.x(), forward.y());
//     auto rotationTransform = AngleAxisd(deltaTheta, Vector3d(0,0,1));
    auto agentTranslation = odometryState->getTransform().translation() - d_lastTranslation;

    d_filter->predict([deltaTheta,&agentTranslation,this](Vector3d const& state) -> Vector3d
                      {
                        AgentPosition pos(state);
                        auto worldAgentTransform = pos.worldAgentTransform();

                        auto worldTranslation = worldAgentTransform.rotation() * agentTranslation;

                        return Vector3d(
                          state.x() + worldTranslation.x() + d_positionErrorRng(),
                          state.y() + worldTranslation.y() + d_positionErrorRng(),
                          state(2) + deltaTheta + d_angleErrorRng());
                     });

  }

  d_lastTranslation = odometryState->getTransform().translation();
  d_lastQuaternion = orientationState->getQuaternion();
}
