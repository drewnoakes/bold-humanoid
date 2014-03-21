#include "spatialiser.ih"

Spatialiser::Spatialiser(shared_ptr<CameraModel> cameraModel)
  : d_cameraModel(std::move(cameraModel))
{
  d_lineJunctionFinder = make_shared<LineJunctionFinder>();
}
