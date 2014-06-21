#pragma once

#include <Eigen/Geometry>
#include <memory>
#include <vector>

#include "../JointId/jointid.hh"
#include "../Math/math.hh"

namespace bold
{
  /**
   * Body part information
   */
  class BodyPart
  {
  public:
    BodyPart(std::string name)
    : name(name),
      rotationOrigin(0)
    {}

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

    /** An offset for the joint's rotation.
     *
     * This is relative to the description in the kinematic model, and exists
     * in cases where the model is simpler to lay out using non-zero hinge
     * positions. For example, the DARwIn-OPs shoulder roll joints push the
     * elbows out to 45 degrees when at zero. The model used describes the
     * robot with arms pointing down, parallel to one another, and the origin
     * for rotation is used to adjust.
     */
    double rotationOrigin;

    /** Get the position of the body part, relative to the torso. */
    Eigen::Vector3d getPosition() const
    {
      return transform.translation().head<3>();
    }

    // Needed when having fixed sized Eigen member
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  };

  class Joint;

  /**
   * Limb information
   */
  class Limb : public BodyPart
  {
  public:
    Limb(std::string name)
    : BodyPart(name)
    {}

    /// Weight of limb
//    double weight;

    /// Weight of limb divided by the robot's total weight
//    double relativeWeight;

    /// Dimensions of limb
//    Eigen::Vector3d size;

    /// Vector of joints attached to this limb
    std::vector<std::shared_ptr<Joint>> joints;

    // Needed when having fixed sized Eigen member
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  };

  /**
   * Joint information
   */
  class Joint : public BodyPart
  {
  public:
    Joint(JointId id, std::string name)
    : BodyPart(name),
      id(id),
      angleRads(0)
    {}

    /// Axis of joint in local coordinate system
    Eigen::Vector3d axis;

    /// This joint's identifier
    JointId id;

//    /// Minimum and maximum angle bounds
//    std::pair<double, double> bounds;

    /// Joint angle in radians
    double angleRads;

    /// Joint angle in degrees (computed from radians.)
    double angleDegs() const { return Math::radToDeg(angleRads); }

//    /// Joint rate
//    double rate;
//
//    /// Torque action upon joint, as given by server
//    double torque;
//
//    /// Control velocity set at last time step
//    double control;

    /// Child part connected by this joint. May be a Limb, or another Joint.
    std::shared_ptr<BodyPart> childPart;

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
