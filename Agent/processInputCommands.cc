#include "agent.ih"

bool isInputAvailable()
{
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
  return (FD_ISSET(0, &fds));
}

void Agent::processInputCommands()
{
  //
  // Control walking via keyboard
  //
  if (isInputAvailable())
  {
    char c;
    std::cin >> c;
    if (c == 'w') {
      d_ambulator->setMoveDir(Eigen::Vector2d(20,0));
    } else if (c == 's') {
      d_ambulator->setMoveDir(Eigen::Vector2d(-20,0));
    } else if (c == 'a') {
      d_ambulator->setMoveDir(Eigen::Vector2d(0,15));
    } else if (c == 'd') {
      d_ambulator->setMoveDir(Eigen::Vector2d(0,-15));
    } else if (c == ',') {
      d_ambulator->setTurnAngle(25);
    } else if (c == '.') {
      d_ambulator->setTurnAngle(-25);
    }
  }

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
            (-axis1/32767.0) * d_joystickXAmpMax,
            (-axis0/32767.0) * d_joystickYAmpMax));
        if (stick == 2)
          d_ambulator->setTurnAngle((-axis2/32767.0) * d_joystickAAmpMax);
      }
      else if (event.isButton() && event.value == 1 && !event.isInitialState())
      {
        switch (event.number)
        {
          case 6:
            cout << "Left kick" << endl;
//             d_actionModule->d_jointData.setEnableBodyWithoutHead(true, true);
            d_actionModule->start((int)ActionPage::KickLeft);
            break;
          case 7:
            cout << "Right kick" << endl;
//             d_actionModule->d_jointData.setEnableBodyWithoutHead(true, true);
            d_actionModule->start((int)ActionPage::KickLeft);
            break;
          default:
            if (event.value == 1)
              cout << "Button " << event.number << endl;
            break;
        }
      }
    }
  }
}
