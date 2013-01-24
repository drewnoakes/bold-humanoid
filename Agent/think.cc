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
//  cout << "[Agent::think] Start" << endl;

  cv::Mat raw;

//  cout << "[Agent::think] Capture image" << endl;
  d_camera >> raw;

  vector<Observation> observations = processImage(raw);

  for (Observation const& obs : observations)
  {
    switch (obs.type)
    {
    case O_BALL:
      cv::circle(raw, cv::Point(obs.pos.x(), obs.pos.y()), 5, cv::Scalar(0,0,255), 2);
      break;

    case O_GOAL_POST:
      cv::circle(raw, cv::Point(obs.pos.x(), obs.pos.y()), 5, cv::Scalar(0,255,255), 2);
      break;
    }

  }

  cv::imshow("raw", raw);

  cv::waitKey(1);

  if (inputAvailable()) {
    char c;
    std::cin >> c;
    if (c == 'w') {
      d_stepTarget = d_stepMax;
    } else if (c == 's') {
      d_stepTarget = -d_stepMax;
    } else if (c == 'a') {
      d_turnTarget = d_turnMax;
    } else if (c == 'd') {
      d_turnTarget = -d_turnMax;
    } else if (c == '[') {
      while(Action::GetInstance()->Start(1) == false) usleep(8000);
      while(Action::GetInstance()->IsRunning() == true) usleep(8000);
    } else if (c == ']') {
      while(Action::GetInstance()->Start(15) == false) usleep(8000);
      while(Action::GetInstance()->IsRunning() == true) usleep(8000);
    }
  }

  auto walk = Walking::GetInstance();

  if (d_stepTarget == 0 && d_turnTarget == 0 && d_stepCurrent == 0 && d_turnCurrent == 0)
  {
    if (walk->IsRunning())
      walk->Stop();
  }
  else
  {
    if (!walk->IsRunning())
    {
      d_stepCurrent = 0;
      d_turnCurrent = 0;
      walk->X_MOVE_AMPLITUDE = d_stepCurrent;
      walk->A_MOVE_AMPLITUDE = d_turnCurrent;
      walk->Start();
      walk->m_Joint.SetEnableBodyWithoutHead(true, true);
    }
    else
    {
      if (d_stepCurrent < d_stepTarget)
        d_stepCurrent += d_stepChangeAmount;
      else if (d_stepCurrent > d_stepTarget)
        d_stepCurrent = d_stepTarget;
      walk->X_MOVE_AMPLITUDE = d_stepCurrent;

      if (d_turnCurrent < d_turnTarget)
        d_turnCurrent += d_turnChangeAmount;
      else if (d_turnCurrent > d_turnTarget)
        d_turnCurrent -= d_turnChangeAmount;
      walk->A_MOVE_AMPLITUDE = d_turnCurrent;

      fprintf(stderr, " (step:%.1f turn:%.1f)\n", d_stepCurrent, d_turnCurrent);
    }
  }
}
