#pragma once

#include <memory>

namespace bold
{
  class BehaviourControl;
  class Debugger;

  class RoleDecider
  {
  public:
    RoleDecider(std::shared_ptr<BehaviourControl> behaviourControl, std::shared_ptr<Debugger> debugger)
    : d_behaviourControl(behaviourControl),
      d_debugger(debugger)
    {}

    void update();

  private:
    std::shared_ptr<BehaviourControl> d_behaviourControl;
    std::shared_ptr<Debugger> d_debugger;
  };
}
