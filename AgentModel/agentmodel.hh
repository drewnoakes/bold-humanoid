#ifndef BOLD_AGENT_MODEL_HH
#define BOLD_AGENT_MODEL_HH

namespace bold
{
  class AgentModel
  {
  private:
    AgentModel()
    : lastThinkCycleMillis(0),
      lastImageCaptureTimeMillis(0),
      lastImageProcessTimeMillis(0)
    {};

    AgentModel(AgentModel const&);
    void operator=(AgentModel const&);

  public:

    double lastThinkCycleMillis;
    double lastImageCaptureTimeMillis;
    double lastImageProcessTimeMillis;

    static AgentModel& getInstance()
    {
      static AgentModel instance;
      return instance;
    }
  };
}

#endif