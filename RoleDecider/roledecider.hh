#pragma once

#include <memory>

namespace bold
{
  class BehaviourControl;
  class Debugger;
  template<typename> class Setting;

  class RoleDecider
  {
  public:
    RoleDecider(std::shared_ptr<BehaviourControl> behaviourControl, std::shared_ptr<Debugger> debugger);

    void update();

  private:
    std::shared_ptr<BehaviourControl> d_behaviourControl;
    std::shared_ptr<Debugger> d_debugger;
    Setting<int>* d_roleOverride;
  };
}
