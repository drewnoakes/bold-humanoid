#pragma once

#include <memory>
#include <string>

class Joystick;

namespace bold
{
  class Agent;
  class WalkModule;
  class HeadModule;
  class MotionScriptModule;
  template<typename> class Setting;

  class RemoteControl
  {
  public:
    RemoteControl(Agent* agent);

    void update();

    std::shared_ptr<Joystick> getJoystick() const { return d_joystick; }

  private:
    void openJoystick();

    Agent* d_agent;
    std::shared_ptr<Joystick> d_joystick;
    std::shared_ptr<MotionScriptModule> d_motionScriptModule;
    std::shared_ptr<HeadModule> d_headModule;
    std::shared_ptr<WalkModule> d_walkModule;

    Setting<bool>* d_joystickEnabled;
    Setting<std::string>* d_joystickDevicePath;
    Setting<double>* d_joystickHeadSpeed;
    Setting<double>* d_joystickXAmpMax;
    Setting<double>* d_joystickYAmpMax;
    Setting<double>* d_joystickAAmpMax;
  };
}
