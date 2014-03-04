#pragma once

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

    void updateZeroGroundPixelTransform(Eigen::Affine3d const& agentCameraTr)
    {
      d_zeroGroundPixelTr = findGroundPixelTransform(agentCameraTr, 0.0);
    }

    Eigen::Matrix3d findGroundPixelTransform(double groundZ = 0.0) const;

    Eigen::Matrix3d findGroundPixelTransform(Eigen::Affine3d const& agentCameraTr,
                                             double groundZ = 0.0) const;

    // TODO rename these two functions to indicate they work in agent space

    /** Returns the ground-plane location, in agent space, for a given camera pixel.
     *
     * @param pixel the x/y pixel location
     */
    Maybe<Eigen::Vector3d> findGroundPointForPixel(Eigen::Vector2d const& pixel, double groundZ = 0) const;

    Maybe<Eigen::Vector3d> findGroundPointForPixel(Eigen::Vector2d const& pixel,
                                                   Eigen::Affine3d const& agentCameraTr,
                                                   double groundZ = 0) const;

    Maybe<Eigen::Vector2d> findPixelForAgentPoint(Eigen::Vector3d const& agentPoint) const;

    Maybe<Eigen::Vector2d> findPixelForAgentPoint(Eigen::Vector3d const& agentPoint,
                                                  Eigen::Affine3d const& agentCameraTr) const;

    int findHorizonForColumn(int column);
    int findHorizonForColumn(int column, Eigen::Affine3d const& agentCameraTr);

    void updateCameraToAgent();

    void updateAgentToWorld(AgentPosition position);

    std::shared_ptr<CameraModel> getCameraModel() const { return d_cameraModel; }


  private:
    Maybe<Eigen::Vector3d> findGroundPointForPixel(Eigen::Vector2d const& pixel, Eigen::Matrix3d const& groundPixelTr) const;

    std::shared_ptr<CameraModel> d_cameraModel;
    Eigen::Matrix3d d_zeroGroundPixelTr;
  };
}
