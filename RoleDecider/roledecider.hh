#pragma once

#include <memory>

namespace bold
{
  class BehaviourControl;
  class Debugger;
  template<typename> class Setting;
  class Voice;

  class RoleDecider
  {
  public:
    RoleDecider(std::shared_ptr<BehaviourControl> behaviourControl, std::shared_ptr<Debugger> debugger, std::shared_ptr<Voice> voice);

    void update();

  private:
    std::shared_ptr<BehaviourControl> d_behaviourControl;
    std::shared_ptr<Debugger> d_debugger;
    std::shared_ptr<Voice> d_voice;
    Setting<int>* d_roleOverride;
    Setting<bool>* d_announceRoles;
  };
}
