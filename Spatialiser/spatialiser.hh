#ifndef BOLD_SPATIALISER_HH
#define BOLD_SPATIALISER_HH

#include <memory>
#include <Eigen/Core>
#include <Eigen/Geometry>

#include "../AgentPosition/agentposition.hh"
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
     *
     * @param pixel the x/y pixel location
     */
    Maybe<Eigen::Vector3d> findGroundPointForPixel(Eigen::Vector2i const& pixel, double const groundZ = 0) const;

    Maybe<Eigen::Vector3d> findGroundPointForPixel(Eigen::Vector2i const& pixel, Eigen::Affine3d const& cameraAgentTransform, double const groundZ = 0) const;

    int findHorizonForColumn(int column, Eigen::Affine3d const& cameraTorsoTransform);

    void updateCameraToAgent();

    void updateAgentToWorld(AgentPosition position);

  private:
    std::shared_ptr<CameraModel> d_cameraModel;
  };
}

#endif
