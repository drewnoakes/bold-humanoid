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
    std::unique_ptr<OptionTree> buildTree(unsigned teamNumber,
                                          unsigned uniformNumber,
                                          Agent* agent,
                                          std::shared_ptr<DataStreamer> dataStreamer,
                                          std::shared_ptr<Debugger> debugger,
                                          std::shared_ptr<CameraModel> cameraModel,
                                          std::shared_ptr<Ambulator> ambulator,
                                          std::shared_ptr<MotionScriptModule> motionScriptModule,
                                          std::shared_ptr<HeadModule> headModule,
                                          std::shared_ptr<WalkModule> walkModule,
                                          std::shared_ptr<FallDetector> fallDetector);
  };
}
