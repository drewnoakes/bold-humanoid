#include "localiser.ih"

pair<Localiser::Filter::State, double> Localiser::generateState()
{
  auto gameState = State::get<GameState>();
  auto behaviourControlState = State::get<BehaviourControlState>();
  bool kidnapped = 
    (gameState && gameState->myPlayerInfo().hasPenalty()) || 
    behaviourControlState->getPlayerStatus() == PlayerStatus::Penalised ||
    behaviourControlState->getPlayerStatus() == PlayerStatus::Paused;

  // If kidnapped, assume we are somewhere on our side of the
  // field, at border, looking in
  // Weight should be probability of being kidnapped
  // (Weight of other particles should actually be multiplied by 1 - this probability)

  if (kidnapped)
  {
    // Pick random side
    bool left = d_fieldYRng() > 0;
    
    // Pick random x; negative = on our side
    // TODO: close to center line is more likely
    auto x = -std::abs(d_fieldXRng());
    // Y is just outside of the field
    // TODO: put a bit of noise on it
    auto y = (left ? -1.0 : 1.0) * (d_fieldMap->fieldLengthY() / 2.0 + 0.5);

    // Assume facing into field
    auto state = Filter::State(x, y, 0, left ? 1.0 : -1.0);
    return make_pair(state, d_penaltyKidnapWeight->getValue());
  }
  else if (gameState && gameState->getPlayMode() != robocup::PlayMode::PLAYING)
  {
    auto theta = -.5 * M_PI + d_thetaRng() / 4;
    auto state = Filter::State(-std::abs(d_fieldXRng()), d_fieldYRng(), cos(theta), sin(theta));

    return make_pair(state, d_defaultKidnapWeight->getValue());
  }
  else
  {
    auto theta = d_thetaRng();
    auto state = Filter::State(d_fieldXRng(), d_fieldYRng(), cos(theta), sin(theta));
        
    return make_pair(state, d_defaultKidnapWeight->getValue());
  }
}
