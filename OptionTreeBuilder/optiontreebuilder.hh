#pragma once

#include <memory>

#include "../OptionTree/optiontree.hh"

class minIni;

namespace bold
{
  class MotionScriptModule;
  class Ambulator;
  class CameraModel;
  class Debugger;
  class HeadModule;
  class WalkModule;
  class FallDetector;

  class OptionTreeBuilder
  {
  public:
    virtual std::unique_ptr<OptionTree> buildTree(unsigned teamNumber,
                                                  unsigned uniformNumber,
                                                  std::shared_ptr<Debugger> debugger,
                                                  std::shared_ptr<CameraModel> cameraModel,
                                                  std::shared_ptr<Ambulator> ambulator,
                                                  std::shared_ptr<MotionScriptModule> motionScriptModule,
                                                  std::shared_ptr<HeadModule> headModule,
                                                  std::shared_ptr<WalkModule> walkModule,
                                                  std::shared_ptr<FallDetector> fallDetector) = 0;
  };
}
