#include "agent.ih"

void Agent::think()
{
  cout << "[Agent::think] Start" << endl;

  cv::Mat raw;
  
  cout << "[Agent::think] Capture image" << endl;
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

}
