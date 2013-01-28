#include "agent.ih"

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
  controlHead(raw, observations);

  //
  // Get up, if we've fallen over
  //
  standUpIfFallen();

  //
  // Process input commands
  //
  processInputCommands();

  d_ambulator.step();

  d_debugger.update(d_CM730);
}
