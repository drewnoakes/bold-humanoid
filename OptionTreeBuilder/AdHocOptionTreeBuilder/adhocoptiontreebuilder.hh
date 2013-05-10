#pragma once

#include "../optiontreebuilder.hh"

namespace bold
{
  class Action;
  class Ambulator;
  class CameraModel;
  class Debugger;
  class Head;
  class WalkModule;

  class AdHocOptionTreeBuilder
  {
  public:
    std::unique_ptr<OptionTree> buildTree(minIni const& ini,
                                          unsigned teamNumber,
                                          unsigned uniformNumber,
                                          bool ignoreGameController,
                                          std::shared_ptr<Debugger> debugger,
                                          std::shared_ptr<CameraModel> cameraModel,
                                          std::shared_ptr<Ambulator> ambulator,
                                          std::shared_ptr<Action> actionModule,
                                          std::shared_ptr<Head> headModule,
                                          std::shared_ptr<WalkModule> walkModule);
  };
}
