#include "agent.ih"

bool inputAvailable()
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

void Agent::think()
{
  //
  // Print out time since last entry to 'think'
  //
  static auto tLast = Debugger::getTimestamp();
  Debugger::printTime(tLast, "Period %4.2f ");
  tLast = Debugger::getTimestamp();

  //
  // Capture the image
  //
  auto t = Debugger::getTimestamp();
  cv::Mat raw;
  d_camera >> raw;
  d_debugger.timeImageCapture(t);

  //
  // Process the image
  //
  t = Debugger::getTimestamp();
  vector<Observation> observations = processImage(raw);
  d_debugger.timeImageProcessing(t);

  //
  // Track ball position with head
  //
  bool foundBall = false;
  Vector2f foundBallAtPx;
  for (Observation const& obs : observations)
  {
    if (obs.type == O_BALL)
    {
      foundBall = true;
      foundBallAtPx = obs.pos;
    }
  }
  if (foundBall)
  {
    float r = 0.9;
    Vector2f centerPx = Vector2f(raw.cols/2, raw.rows/2);
    Vector2f offset = (foundBallAtPx - centerPx) * r;
    offset.x() *= (Camera::VIEW_H_ANGLE / (double)raw.cols); // pixel per angle
    offset.y() *= (Camera::VIEW_V_ANGLE / (double)raw.rows); // pixel per angle
//    std::cout << "Found ball at " << foundBallAtPx.x() << "," << foundBallAtPx.y() << " - "
//              << "Corresponds to an offset of " << offset.x() << "," << offset.y() << std::endl;
    Head::GetInstance()->MoveTracking(Point2D(offset.x(), offset.y()));
  }
  d_debugger.setIsBallObserved(foundBall);

  //
  // Get up, if we've fallen over
  //
  if (d_autoGetUpFromFallen && MotionStatus::FALLEN != STANDUP)
  {
    Walking::GetInstance()->Stop();
    while(Walking::GetInstance()->IsRunning() == 1) usleep(8000);

    Robot::Action::GetInstance()->m_Joint.SetEnableBody(true, true);

    if (MotionStatus::FALLEN == FORWARD)
      Robot::Action::GetInstance()->Start(10);   // FORWARD GETUP
    else if (MotionStatus::FALLEN == BACKWARD)
      Robot::Action::GetInstance()->Start(11);   // BACKWARD GETUP

    while (Robot::Action::GetInstance()->IsRunning() == 1) usleep(8000);

    Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);
    Walking::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true, true);
  }

  //
  // Control walking via keyboard
  //
  if (inputAvailable())
  {
    char c;
    std::cin >> c;
    if (c == 'w') {
      d_ambulator.setMoveDir(Eigen::Vector2d(20,0));
    } else if (c == 's') {
      d_ambulator.setMoveDir(Eigen::Vector2d(-20,0));
    } else if (c == 'a') {
      d_ambulator.setMoveDir(Eigen::Vector2d(0,15));
    } else if (c == 'd') {
      d_ambulator.setMoveDir(Eigen::Vector2d(0,-15));
    } else if (c == ',') {
      d_ambulator.setTurnAngle(25);
    } else if (c == '.') {
      d_ambulator.setTurnAngle(-25);
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
      static short int axis3 = 0;

      // TODO what could the buttons be used for?
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
          case 3:
            axis3 = event.value;
            stick = 2;
            break;
        }

        if (stick == 1)
          d_ambulator.setMoveDir(Eigen::Vector2d((-axis1/32767.0) * 15, (-axis0/32767.0) * 15));
        if (stick == 2)
          d_ambulator.setTurnAngle((-axis2/32767.0) * 15);
      }
      else if (event.isButton() && event.value == 1 && !event.isInitialState())
      {
        switch (event.number)
        {
          // Action 10 -> Forward get up
          // Action 11 -> Backward get up
          // Action 12 -> Right kick
          // Action 13 -> Left kick
          case 6:
            cout << "Left kick" << endl;
            Robot::Action::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true, true);
            Robot::Action::GetInstance()->Start(13);
            break;
          case 7:
            cout << "Right kick" << endl;
            Robot::Action::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true, true);
            Robot::Action::GetInstance()->Start(12);
            break;
          default:
            if (event.value == 1)
              printf("Button %u\n", event.number);
            break;
        }
      }
    }
  }

  d_ambulator.step();

  d_debugger.update(d_CM730);
}
