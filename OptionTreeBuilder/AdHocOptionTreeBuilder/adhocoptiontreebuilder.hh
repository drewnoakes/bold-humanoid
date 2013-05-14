#pragma once

#include "../optiontreebuilder.hh"

namespace bold
{
  class Debugger;
  class CameraModel;
  class Ambulator;

  class AdHocOptionTreeBuilder
  {
  public:
    std::unique_ptr<OptionTree> buildTree(unsigned teamNumber,
                                          unsigned uniformNumber,
                                          bool ignoreGameController,
                                          std::shared_ptr<Debugger> debugger,
                                          std::shared_ptr<CameraModel> cameraModel,
                                          std::shared_ptr<Ambulator> ambulator);
  };
}
