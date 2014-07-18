#pragma once

#include "../option.hh"
#include "../../StateObject/GameState/gamestate.hh"

namespace bold
{
  class MotionScriptModule;
  class MotionScriptOption;
  class Voice;

  class GameOver : public Option
  {
  public:
    GameOver(std::string const& id, std::shared_ptr<MotionScriptModule> motionScriptModule, std::shared_ptr<Voice> voice);

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    virtual void reset() override;

    virtual double hasTerminated() override;

  private:
    std::shared_ptr<MotionScriptOption> d_motionScriptOption;
    std::shared_ptr<Voice> d_voice;
    GameResult d_result;
    bool d_isScriptPlaying;
    uint d_playCount;
    bool d_hasTerminated;
  };
}
