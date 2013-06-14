#include "agent.ih"

void Agent::run()
{
  cout << "[Agent::run] Start" << endl;

  if (d_isRunning)
    throw new std::runtime_error("Already running");

  d_isRunning = true;

  if (d_haveBody)
  {
    d_cm730->torqueEnable(true);
  
    d_motionLoop->start();

    cout << "[Agent::run] Waiting for HardwareState" << endl;
    while (!AgentState::get<HardwareState>())
    {
      // Wait until the motion loop has read a hardware value
      usleep(8000);
    }
    
    // TODO move this to an initialisation phase of the behaviour tree
    cout << "[Agent::run] Getting into initial pose" << endl;
    auto sit = d_optionTree->getOption("sitdownaction");
    cout << "sit: " << (sit ? "yes" : "no") << endl;

    while (sit->hasTerminated() == 0.0)
    {
      sit->runPolicy();
      usleep(8000);
    }
  }
  
  cout << "[Agent::run] Starting think loop" << endl;

  while (d_isRunning)
  {
    think();
  }

  // TODO if the agent is walking, allow it to come to a stable pose before terminating

  cout << "[Agent::run] Stopped" << endl;
}
