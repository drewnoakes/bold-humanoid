#include "visualcortex.ih"

VisualCortex::VisualCortex(shared_ptr<CameraModel> cameraModel, shared_ptr<FieldMap> fieldMap, shared_ptr<Debugger> debugger)
  : d_cameraModel(cameraModel),
    d_debugger(debugger),
    d_fieldMap(fieldMap),
    d_minBallArea(8*8)
{}
