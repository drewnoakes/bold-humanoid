#include "localiser.ih"

void Localiser::predict()
{
  auto orientationState = State::get<OrientationState>(StateTime::CameraImage);
  auto odometryState = State::get<OdometryState>(StateTime::CameraImage);
  if (!orientationState || !odometryState)
    return;

  if (d_haveLastAgentTransform)
  {
    auto deltaAgentTransform = odometryState->getTransform() * d_lastAgentTransform.inverse();

    auto deltaAgentMat = deltaAgentTransform.matrix();

    d_filter->predict([deltaAgentMat,this](FilterState const& state) -> FilterState
                      {
                        FilterState newState;
                        newState <<
                          (deltaAgentMat.block<2,2>(0,0) * state.head<2>() + deltaAgentMat.col(3).head<2>()),
                          deltaAgentMat.block<2,2>(0,0) * state.tail<2>();

                        newState(0) += d_positionErrorRng();
                        newState(1) += d_positionErrorRng();

                        auto theta = atan2(newState(3), newState(2));
                        theta += d_angleErrorRng();
                        newState(3) = sin(theta);
                        newState(2) = cos(theta);
                        return newState;
                      });
    // New state:
    // Location update: 
    /*
    auto deltaQuaternion = orientationState->getQuaternion() * d_lastQuaternion.inverse();
    auto transformMatrix = deltaQuaternion.matrix();

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
    */

  }

  d_lastAgentTransform = odometryState->getTransform();
  d_haveLastAgentTransform = true;
  d_lastQuaternion = orientationState->getQuaternion();
}
