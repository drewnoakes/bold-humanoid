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
  cv::Mat raw;

//  cout << "[Agent::think] Capture image" << endl;
  d_camera >> raw;

  vector<Observation> observations = processImage(raw);

  if (d_showUI)
  {
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
  }

  if (inputAvailable())
  {
    char c;
    std::cin >> c;
    if (c == 'w') {
      d_ambulator.setMoveDir(Eigen::Vector2d(1,0));
    } else if (c == 's') {
      d_ambulator.setMoveDir(Eigen::Vector2d(-1,0));
    } else if (c == 'a') {
      d_ambulator.setMoveDir(Eigen::Vector2d(0,1));
    } else if (c == 'd') {
      d_ambulator.setMoveDir(Eigen::Vector2d(0,-1));
    } else if (c == ',') {
      d_ambulator.setTurnAngle(-1);
    } else if (c == '.') {
      d_ambulator.setTurnAngle(1);
    }
  }
}
