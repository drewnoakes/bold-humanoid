#include "agent.ih"

#include "../MotionScriptRunner/motionscriptrunner.hh"

void Agent::processInputCommands()
{
  if (d_joystick == nullptr || !d_joystick->isFound())
    return;

  //
  // Control via joystick
  //

  static bool isButton1Down = false;
  static short int axis0 = 0;
  static short int axis1 = 0;
  static short int axis2 = 0;
//static short int axis3 = 0;

  //
  // Process any new joystick events, updating state
  //

  JoystickEvent event;
  while (d_joystick->sample(&event))
  {
    if (event.isAxis())
    {
      switch (event.number)
      {
        case 0:
          axis0 = event.value;
          break;
        case 1:
          axis1 = event.value;
          break;
        case 2:
          axis2 = event.value;
          break;
//      case 3:
//        axis3 = event.value;
//        break;
      }
    }
    else if (event.isButton() && event.number == 1)
    {
      isButton1Down = event.value == 1;
    }
    else if (event.isButton() && event.value == 1 && !event.isInitialState())
    {
      static auto leftKickScript = make_shared<MotionScriptRunner>(MotionScript::fromFile("./motionscripts/kick-left.json"));
      static auto rightKickScript = make_shared<MotionScriptRunner>(MotionScript::fromFile("./motionscripts/kick-right.json"));

      switch (event.number)
      {
        case 6:
          log::info("Agent::processInputCommands") << "Left kick";
          d_motionScriptModule->start(leftKickScript);
          break;
        case 7:
          log::info("Agent::processInputCommands") << "Right kick";
          d_motionScriptModule->start(rightKickScript);
          break;
        default:
          if (event.value == 1)
            log::info("Agent::processInputCommands") << "Button " << (int)event.number;
          break;
      }
    }
  }

  //
  // Update based upon state
  //

  if (axis0 != 0 || axis1 != 0)
  {
    if (isButton1Down)
    {
      d_headModule->moveByDeltaDegs(
        (-axis0/32767.0) * 5,
        (-axis1/32767.0) * 5);
    }
    else
    {
      d_ambulator->setMoveDir(Eigen::Vector2d(
        (-axis1/32767.0) * d_joystickXAmpMax->getValue(),
        (-axis0/32767.0) * d_joystickYAmpMax->getValue()));
    }
  }

  if (axis2 != 0)
    d_ambulator->setTurnAngle((-axis2/32767.0) * d_joystickAAmpMax->getValue());
}
