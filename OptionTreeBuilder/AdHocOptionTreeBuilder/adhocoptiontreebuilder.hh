#pragma once

#include "../optiontreebuilder.hh"

namespace bold
{
  class ActionModule;
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
                                          bool ignoreGameController,
                                          std::shared_ptr<Debugger> debugger,
                                          std::shared_ptr<CameraModel> cameraModel,
                                          std::shared_ptr<Ambulator> ambulator,
                                          std::shared_ptr<ActionModule> actionModule,
                                          std::shared_ptr<HeadModule> headModule,
                                          std::shared_ptr<WalkModule> walkModule);
  };
}
