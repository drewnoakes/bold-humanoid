#include "agent.ih"

#include "../MotionScriptRunner/motionscriptrunner.hh"

void Agent::processInputCommands()
{
  //
  // Control via joystick
  //
  if (d_joystick != nullptr)
  {
    JoystickEvent event;
    while (d_joystick->sample(&event))
    {
      static short int axis0 = 0;
      static short int axis1 = 0;
      static short int axis2 = 0;
//       static short int axis3 = 0;

      // what could the buttons be used for?
      if (event.isAxis())
      {
        int stick = -1;
        switch (event.number)
        {
          case 0:
            axis0 = event.value;
            stick = 1;
            break;
          case 1:
            axis1 = event.value;
            stick = 1;
            break;
          case 2:
            axis2 = event.value;
            stick = 2;
            break;
//           case 3:
//             axis3 = event.value;
//             stick = 2;
//             break;
        }

        if (stick == 1)
          d_ambulator->setMoveDir(Eigen::Vector2d(
            (-axis1/32767.0) * d_joystickXAmpMax->getValue(),
            (-axis0/32767.0) * d_joystickYAmpMax->getValue()));

        if (stick == 2)
          d_ambulator->setTurnAngle((-axis2/32767.0) * d_joystickAAmpMax->getValue());
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
              log::info("Agent::processInputCommands") << "Button " << event.number;
            break;
        }
      }
    }
  }
}
