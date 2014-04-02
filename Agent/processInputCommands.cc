#include "agent.ih"

#include "../MotionScriptRunner/motionscriptrunner.hh"

void Agent::processInputCommands()
{
  if (d_joystick == nullptr || !d_joystick->isFound())
    return;

  static auto leftKickScript = MotionScript::fromFile("./motionscripts/kick-left.json");
  static auto rightKickScript = MotionScript::fromFile("./motionscripts/kick-right.json");
  static auto leftSideKickScript = MotionScript::fromFile("./motionscripts/kick-side-left.json");
  static auto rightSideKickScript = MotionScript::fromFile("./motionscripts/kick-side-right.json");
  static auto standReadyScript = MotionScript::fromFile("./motionscripts/stand-ready.json");
  static auto sitDownScript = MotionScript::fromFile("./motionscripts/sit-down.json");

  //
  // Control via joystick
  //

  static bool isButton1Down = false;
  static short int axis0 = 0;
  static short int axis1 = 0;
  static short int axis2 = 0;
  static short int axis3 = 0;
  static short int axis4 = 0;
  static short int axis5 = 0;

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
        case 0: axis0 = event.value; break;
        case 1: axis1 = event.value; break;
        case 2: axis2 = event.value; break;
        case 3: axis3 = event.value; break;
        case 4: axis4 = event.value; break;
        case 5: axis5 = event.value; break;
        default:
          log::info("Agent::processInputCommands") << "Axis " << (int)event.number << " value " << (int)event.value;
          break;
      }
    }
    else if (event.isButton() && event.number == 1)
    {
      isButton1Down = event.value == 1;
    }
    else if (event.isButton() && event.value == 1 && !event.isInitialState())
    {
      auto runIfStanding = [this,standReadyScript](shared_ptr<MotionScript const> const& script)
      {
        bool isStanding = MotionScriptRunner::isInFinalPose(standReadyScript);
        if (isStanding)
          d_motionScriptModule->start(make_shared<MotionScriptRunner>(script));
      };

      switch (event.number)
      {
        case 4: runIfStanding(leftSideKickScript);  break;
        case 5: runIfStanding(rightSideKickScript); break;
        case 6: runIfStanding(leftKickScript);      break;
        case 7: runIfStanding(rightKickScript);     break;
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
        (-axis0/32767.0) * d_joystickHeadSpeed->getValue(),
        (-axis1/32767.0) * d_joystickHeadSpeed->getValue());
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

  if (axis5 < 0 && !MotionScriptRunner::isInFinalPose(standReadyScript))
    d_motionScriptModule->start(make_shared<MotionScriptRunner>(standReadyScript));
  else if (axis5 > 0 && !MotionScriptRunner::isInFinalPose(sitDownScript))
    d_motionScriptModule->start(make_shared<MotionScriptRunner>(sitDownScript));
}
