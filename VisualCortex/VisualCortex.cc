#include "visualcortex.ih"

VisualCortex::VisualCortex(shared_ptr<CameraModel> cameraModel, std::shared_ptr<FieldMap> fieldMap)
  : d_cameraModel(cameraModel),
    d_fieldMap(fieldMap),
    d_minBallArea(8*8)
{}
