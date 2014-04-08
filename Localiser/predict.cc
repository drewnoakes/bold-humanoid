#include "localiser.ih"

void Localiser::predict()
{
  if (d_shouldRandomise)
  {
    if (d_filterType == FilterType::Particle)
    {
      auto filter = static_pointer_cast<ParticleFilterUsed>(d_filter);
      filter->randomise();
      filter->normalize();
      d_shouldRandomise = false;
    }
  }

  auto orientationState = State::get<OrientationState>(StateTime::CameraImage);
  auto odometryState = State::get<OdometryState>(StateTime::CameraImage);
  if (!orientationState || !odometryState)
    return;

  if (d_haveLastAgentTransform)
  {
    // Particle represents WA
    // Predict new A': WA' = WA * AA'
    auto deltaAgentTransform = odometryState->getTransform() * d_lastAgentTransform.inverse();

    auto deltaAgentMat4 = deltaAgentTransform.matrix();
    Matrix3d deltaAgentMat;
    deltaAgentMat <<
      deltaAgentMat4.block<2,2>(0,0) , deltaAgentMat4.col(3).head<2>(),
      0, 0, 1;

    d_filter->predict(
      [deltaAgentMat,this](FilterState const& state) -> FilterState
      {
        Matrix3d worldAgentMat;
        worldAgentMat <<
          state(3), state(2), state.x(),
          -state(2), state(3), state.y(),
          0        , 0       , 1;
        
        
        auto newWorldAgentMat = worldAgentMat * deltaAgentMat;
        
        FilterState newState;
        newState <<
          newWorldAgentMat.col(2).head<2>(), newWorldAgentMat.col(1).head<2>();
        
        newState(0) += d_positionErrorRng();
        newState(1) += d_positionErrorRng();
        
        auto theta = atan2(newState(3), newState(2));
        theta += d_angleErrorRng();
        newState(3) = sin(theta);
        newState(2) = cos(theta);
        return newState;
      });
  }

  d_lastAgentTransform = odometryState->getTransform();
  d_haveLastAgentTransform = true;
  d_lastQuaternion = orientationState->getQuaternion();
}
