#include "localiser.ih"

template<int DIM>
class DarwinMotionModel : public GaussianMotionModel<DIM>
{
public:
  using State = typename GaussianMotionModel<DIM>::State;

  DarwinMotionModel()
  {
    auto positionError  = Config::getSetting<double>("localiser.position-error");
    auto angleErrorDegs = Config::getSetting<double>("localiser.angle-error-degrees");
    positionError->track([this](double value) { d_positionErrorRng = Math::createNormalRng(0, value); });
    angleErrorDegs->track([this](double value) { d_angleErrorRng = Math::createNormalRng(0, Math::degToRad(value)); });
  }

  void setDeltaAgentMat(Eigen::Matrix3d mat)
  {
    d_deltaAgentMat = mat;
  }

  State operator()(State const& state) const override
  {
    Matrix3d worldAgentMat;
    worldAgentMat <<
      state(3), state(2), state.x(),
      -state(2), state(3), state.y(),
      0        , 0       , 1;
        
    Matrix3d newWorldAgentMat = worldAgentMat * d_deltaAgentMat;
        
    State newState;
    newState <<
      newWorldAgentMat.col(2).head<2>(), newWorldAgentMat.col(1).head<2>();

    return newState;
  }

  State perturb(State const& state) const override
  {
    auto newState = state;

    auto xPosErr = d_positionErrorRng();
    auto yPosErr = d_positionErrorRng();
    
    /*
    if (d_dynamicError)
    {
      auto alpha = (1.0 - d_preNormWeightSumFilter.getValue()) / 0.1;
      Math::clamp(alpha, 0.0, 1.0);
      xPosErr *= alpha;
      yPosErr *= alpha;
      
    }
    */
    newState(0) += xPosErr;
    newState(1) += yPosErr;
    
    auto theta = atan2(newState(3), newState(2));
    theta += d_angleErrorRng();
    newState(3) = sin(theta);
    newState(2) = cos(theta);

    return newState;
  }

private:
  std::function<double()> d_positionErrorRng;
  std::function<double()> d_angleErrorRng;

  Eigen::Matrix3d d_deltaAgentMat;
  bool d_dynamicError;
};

void Localiser::predict()
{
  if (d_shouldRandomise)
  {
    if (d_filterType == FilterType::Particle)
    {
      d_filter->reset(generateState().first);
      d_filter->endStep();
      d_shouldRandomise = false;
    }
  }

  auto orientationState = State::get<OrientationState>(StateTime::CameraImage);
  auto odometryState = State::get<OdometryState>(StateTime::CameraImage);
  if (!orientationState || !odometryState)
    return;

  DarwinMotionModel<4> motionModel;
//  bool dynamicError = d_enableDynamicError->getValue();

  if (d_haveLastAgentTransform)
  {
    // Particle represents WA
    // Odometer gives AA0
    // Predict new A': WA' = WA * AA' = WA * AA0 * A0A'
    auto deltaAgentTransform = d_lastAgentTransform * odometryState->getTransform().inverse();

    auto deltaAgentMat4 = deltaAgentTransform.matrix();
    Matrix3d deltaAgentMat;
    deltaAgentMat <<
      deltaAgentMat4.block<2,2>(0,0) , deltaAgentMat4.col(3).head<2>(),
      0, 0, 1;

    motionModel.setDeltaAgentMat(deltaAgentMat);
    d_filter->predict(motionModel);
  }

  d_lastAgentTransform = odometryState->getTransform();
  d_haveLastAgentTransform = true;
  d_lastQuaternion = orientationState->getQuaternion();
}
