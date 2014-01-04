#pragma once

#include <Eigen/Geometry>
#include <memory>
#include <vector>

#include "../JointId/jointid.hh"

namespace bold
{
  /**
   * Body part information
   */
  struct BodyPart
  {
    virtual ~BodyPart() {}

    /// Name of body part
    std::string name;

    /** The transformation matrix of this body part.
     *
     * This matrix contains the position and orientation of the body part
     * relative to the torso.
     *
     * Using the returned Affine3d to transform a vector will convert
     * from the frame of this body part into the frame of the torso.
     */
    Eigen::Affine3d transform;

    Eigen::Vector3d getPosition()
    {
      return transform.translation().head<3>();
    }

    // Needed when having fixed sized Eigen member
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  };

  struct Joint;

  /**
   * Limb information
   */
  struct Limb : public BodyPart
  {
    /// Weight of limb
    double weight;

    /// Weight of limb divided by the robot's total weight
    double relativeWeight;

    /// Dimensions of limb
    Eigen::Vector3d size;

    /// List of joints attached to this limb
    std::vector<std::shared_ptr<Joint>> joints;

    // This bodypart's identifier
    unsigned id;

    // Needed when having fixed sized Eigen member
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  };

  /**
   * Joint information
   */
  struct Joint : public BodyPart
  {
    Joint()
    : angleRads(0)
//    rate(0), torque(0), control(0)
    {}

    /// Axis of joint in local coordinate system
    Eigen::Vector3d axis;

    /// This joint's identifier
    JointId id;

//    /// Minimum and maximum angle bounds
//    std::pair<double, double> bounds;

    /// Joint angle in radians
    double angleRads;

//    /// Joint rate
//    double rate;
//
//    /// Torque action upon joint, as given by server
//    double torque;
//
//    /// Control velocity set at last time step
//    double control;

    /// Body part connected by this joint
    std::shared_ptr<BodyPart> bodyPart;

    /// Anchor points of joint on body parts, relative to their center
    std::pair<Eigen::Vector3d, Eigen::Vector3d> anchors;

    /// @returns the joint's axis direction vector in the agent coordinate system
    Eigen::Vector3d getAxisVec() const
    {
      return transform * axis;
    }

    // Needed when having fixed sized Eigen member
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  };
}
