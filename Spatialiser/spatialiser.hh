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

    void updateZeroGroundPixelTransform();

    void updateZeroGroundPixelTransform(Eigen::Affine3d const& agentCameraTr);

    Eigen::Matrix3d getZeroGroundPixelTransform() const { return d_zeroGroundPixelTr; }

    Eigen::Matrix3d findGroundPixelTransform(double groundZ = 0.0) const;

    Eigen::Matrix3d findGroundPixelTransform(Eigen::Affine3d const& agentCameraTr,
                                             double groundZ = 0.0) const;

    // TODO rename these two functions to indicate they work in agent space

    /** Returns the ground-plane location, in agent space, for a given
     * camera pixel.
     *
     * @param pixel The x/y pixel location
     * @param groundZ The z coordinate of the ground plane that is used; default is 0
     */
    Maybe<Eigen::Vector3d> findGroundPointForPixel(Eigen::Vector2d const& pixel, double groundZ = 0.0) const;

    /** Returns the ground-plane location, in agent space, for a given
     * camera pixel and camera transformation.
     *
     * @param pixel The x/y pixel location
     * @param agentCameraTr The transformation from camera to agent frame
     * @param groundZ The z coordinate of the ground plane that is used; default is 0
     */
    Maybe<Eigen::Vector3d> findGroundPointForPixel(Eigen::Vector2d const& pixel,
                                                   Eigen::Affine3d const& agentCameraTr,
                                                   double groundZ = 0) const;


    /** Returns the ground-plane locations, in agent space, for a
     * given set of camera pixels.
     *
     * @param pixels A 2xN matrix, where each column denotes a pixel coordinate
     * @param groundZ The z coordinate of the ground plane that is used; default is 0
     * @returns a pair containing a 3xN matrix of ground locations and
     * an N-dimensional vector that denotes whether these are valid
     */
    std::pair<Eigen::MatrixXd,Eigen::VectorXi> findGroundPountsForPixels(Eigen::MatrixXd const& pixels,
                                                                         double groundZ = 0.0) const;

    /** Returns the ground-plane locations, in agent space, for a
     * given set of camera pixels and camera transfrmation.
     *
     * @param pixels A 2xN matrix, where each column denotes a pixel coordinate
     * @param agentCameraTr The transformation from camera to agent frame
     * @param groundZ The z coordinate of the ground plane that is used; default is 0
     */
    std::pair<Eigen::MatrixXd,Eigen::VectorXi> findGroundPountsForPixels(Eigen::MatrixXd const& pixels,
                                                                         Eigen::Affine3d const& agentCameraTr,
                                                                         double groundZ = 0.0) const;

    Maybe<Eigen::Vector2d> findPixelForAgentPoint(Eigen::Vector3d const& agentPoint) const;

    Maybe<Eigen::Vector2d> findPixelForAgentPoint(Eigen::Vector3d const& agentPoint,
                                                  Eigen::Affine3d const& agentCameraTr) const;

    int findHorizonForColumn(int column);
    int findHorizonForColumn(int column, Eigen::Affine3d const& agentCameraTr);

    void updateCameraToAgent();

    void updateAgentToWorld(AgentPosition position);

    std::shared_ptr<CameraModel> getCameraModel() const { return d_cameraModel; }


  private:
    Maybe<Eigen::Vector3d> findGroundPointForPixel(Eigen::Vector2d const& pixel,
                                                   Eigen::Matrix3d const& groundPixelTr) const;
    std::pair<Eigen::MatrixXd, Eigen::VectorXi> findGroundPountsForPixels(Eigen::MatrixXd const& pixels,
                                                                          Eigen::Matrix3d const& groundPixelTr) const;

    std::shared_ptr<CameraModel> d_cameraModel;
    Eigen::Matrix3d d_zeroGroundPixelTr;
  };
}
