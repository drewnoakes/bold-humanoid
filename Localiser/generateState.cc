#include "localiser.ih"

pair<Localiser::FilterState, double> Localiser::generateState()
{
  auto gameState = State::get<GameState>();
  auto behaviourControlState = State::get<BehaviourControlState>();

  PlayerRole role = behaviourControlState->getPlayerRole();

  if (role == PlayerRole::Keeper)
  {
    // Generate inside the penalty area
    auto x = d_goalAreaXRng();
    auto y = d_goalAreaYRng();
//    cout << x << " " << y << endl;
    auto theta = d_thetaRng();
    auto state = FilterState(x, y, cos(theta), sin(theta));

    return make_pair(state, d_defaultKidnapWeight->getValue());
  }

    bool kidnapped = d_enablePenaltyRandomise->getValue() &&
      ((gameState && gameState->getMyPlayerInfo().hasPenalty()) ||
       behaviourControlState->getPlayerStatus() == PlayerStatus::Penalised ||
       behaviourControlState->getPlayerStatus() == PlayerStatus::Paused);

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
      auto y = (left ? -1.0 : 1.0) * (FieldMap::getFieldLengthY() / 2.0 + 0.5);

      // Assume facing into field
      auto state = FilterState(x, y, 0, left ? 1.0 : -1.0);
      return make_pair(state, d_penaltyKidnapWeight->getValue());
    }
    else if (gameState && gameState->getPlayMode() != PlayMode::PLAYING)
    {
      auto theta = -.5 * M_PI + d_thetaRng() / 4;
      auto state = FilterState(-std::abs(d_fieldXRng()), d_fieldYRng(), cos(theta), sin(theta));

      return make_pair(state, d_defaultKidnapWeight->getValue());
    }
    else
    {
      auto theta = d_thetaRng();
      auto state = FilterState(d_fieldXRng(), d_fieldYRng(), cos(theta), sin(theta));

      return make_pair(state, d_defaultKidnapWeight->getValue());
    }
}
