#ifndef BOLD_SPATIALISER_HH
#define BOLD_SPATIALISER_HH

#include <memory>
#include <Eigen/Core>
#include <Eigen/Geometry>

#include "../CameraModel/cameramodel.hh"
#include "../util/Maybe.hh"

namespace bold
{
  class Spatialiser
  {
  public:
    Spatialiser(std::shared_ptr<CameraModel> cameraModel)
    : d_cameraModel(cameraModel)
    {}

    /** Returns the ground-plane location, in agent space, for a given camera pixel.
     * Assumes that the torso is vertical.
     *
     * @param pixel the x/y pixel location
     * @param torsoHeight the height of the torso from the ground plane
     * @param cameraTransform the transformation from camera to torso frame
     */
    Maybe<Eigen::Vector3d> findGroundPointForPixel(Eigen::Vector2i const& pixel, double const torsoHeight, Eigen::Affine3d const& cameraTorsoTransform) const;

  private:
    std::shared_ptr<CameraModel> d_cameraModel;
  };
}

#endif