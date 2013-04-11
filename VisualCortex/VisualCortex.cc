#include "visualcortex.ih"

VisualCortex::VisualCortex(shared_ptr<CameraModel> cameraModel)
  : d_cameraModel(cameraModel),
    d_minBallArea(8*8)
{}
