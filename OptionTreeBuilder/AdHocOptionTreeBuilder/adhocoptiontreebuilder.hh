#pragma once

#include "../optiontreebuilder.hh"

namespace bold
{
  class DataStreamer;
  class MotionScriptModule;
  class Agent;
  class Ambulator;
  class CameraModel;
  class Debugger;
  class HeadModule;
  class WalkModule;

  class AdHocOptionTreeBuilder
  {
  public:
    std::shared_ptr<OptionTree> buildTree(Agent* agent);
  };
}
