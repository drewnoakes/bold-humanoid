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
    : name(name)
    {}

    virtual ~BodyPart() = default;

    /// Name of body part
    std::string name;

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

    /// Mass of limb, in KG
    double mass;

    // Position of the centre of mass
    Eigen::Vector3d com;

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
      rotationOrigin(0)
    {}

    /// This joint's identifier
    JointId id;

    /// Axis of joint in local coordinate system
    Eigen::Vector3d axis;

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

//    /// Minimum and maximum angle bounds
//    std::pair<double, double> bounds;

    /// Child part connected by this joint. May be a Limb, or another Joint.
    std::shared_ptr<BodyPart const> childPart;

    /// Anchor points of joint on body parts, relative to their center
    std::pair<Eigen::Vector3d,Eigen::Vector3d> anchors;

    // Needed when having fixed sized Eigen member
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  };
}
