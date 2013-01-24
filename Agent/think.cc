#include "agent.ih"

void Agent::think()
{
  cout << "[Agent::think] Start" << endl;

  cv::Mat raw;
  
  cout << "[Agent::think] Capture image" << endl;
  d_camera >> raw;
}
