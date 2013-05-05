%module bold
%{
#include "../Agent/agent.hh"
#include "../OptionTree/optiontree.hh"
%}

%include "std_string.i"

namespace bold
{
  class Agent
  {
  public:
    Agent(std::string const& U2D_dev,
          std::string const& confFile,
          std::string const& motionFile,
          unsigned teamNumber,
          unsigned uniformNumber,
          bool useJoystick,
          bool autoGetUpFromFallen,
          bool useOptionTree,
          bool recordFrames,
          bool ignoreGameController);

    void run();
    void stop();
  };
}
