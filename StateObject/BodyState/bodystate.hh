#pragma once

#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

#include "../stateobject.hh"
#include "../../BodyPart/bodypart.hh"
#include "../../JointId/jointid.hh"
#include "../../util/assert.hh"
#include "../../util/log.hh"

namespace bold
{
  typedef unsigned char uchar;

  class BodyPartPosition
  {
  public:
    /** The transformation matrix of this body part.
    *
    * This matrix contains the position and orientation of the body part
    * relative to the torso.
    *
    * Using the returned Affine3d to transform a vector will convert
    * from the frame of this body part into the frame of the torso.
    */
    Eigen::Affine3d const& getTransform() const { return d_transform; }

    /** Get the position of the body part, relative to the torso. */
    Eigen::Vector3d getPosition() const
    {
      return d_transform.translation().head<3>();
    }

  protected:
    BodyPartPosition(Eigen::Affine3d const& transform)
    : d_transform(transform)
    {}

    virtual ~BodyPartPosition() = default;

    // TODO rename partTorsoTransform (and check that's correct!)
    Eigen::Affine3d d_transform;
  };

  class LimbPosition : public BodyPartPosition
  {
  public:
    LimbPosition(std::shared_ptr<Limb const> limb, Eigen::Affine3d const& transform)
    : BodyPartPosition(transform),
      d_limb(limb)
    {
      ASSERT(limb);
    }

    std::shared_ptr<Limb const> const& getLimb() const { return d_limb; }

  private:
    std::shared_ptr<Limb const> d_limb;
  };

  class JointPosition : public BodyPartPosition
  {
  public:
    JointPosition(std::shared_ptr<Joint const> joint, Eigen::Affine3d const& transform, double angleRads)
    : BodyPartPosition(transform),
      d_joint(joint),
      d_angleRads(angleRads)
    {
      ASSERT(joint);
    }

    std::shared_ptr<Joint const> const& getJoint() const { return d_joint; }

    /// @returns the joint's axis direction vector in the agent coordinate system
    Eigen::Vector3d getAxisVec() const { return d_transform * d_joint->axis; }

    double getAngleRads() const { return d_angleRads; }
    double getAngleDegs() const { return Math::radToDeg(d_angleRads); }

  private:
    std::shared_ptr<Joint const> d_joint;
    double d_angleRads;
  };

  class BodyControl;
  class BodyModel;
  class HardwareState;
  template<typename> class Setting;

  /** Models the kinematic chain of the robot's body.
   *
   * The coordinate frame of body parts has x pointing to the right,
   * y pointing forwards, and z pointing up, relative to the agent's forward
   * facing direction.
   *
   * The body is modelled using subclasses of BodyPart:
   * - Limb represents a rigid body part
   * - Joint represents a single revolute joint that connects two body parts
   *
   * The torso is the root of the kinematic chain, from which five chains
   * extend: the two legs, two arms and the head.
   *
   * All body parts know their transform, which contains the translation and
   * rotation of the part's coordinate frame, in the frame of the torso.
   *
   * For more detail on the naming and use of transforms, see EigenTests.hh.
   */
  class BodyState : public StateObject
  {
  public:
    static std::shared_ptr<BodyState const> zero(std::shared_ptr<BodyModel const> const& bodyModel, ulong thinkCycleNumber = 0);

    static Eigen::Vector3d distanceBetween(std::shared_ptr<BodyPartPosition const> const& p1, std::shared_ptr<BodyPartPosition const> const& p2)
    {
      return p2->getPosition() - p1->getPosition();
    }

    /// Initialise with the specified angles, in radians. Indexed by JointId (i.e. 0 is ignored.)
    BodyState(
      std::shared_ptr<BodyModel const> const& bodyModel,
      std::array<double,23> const& angles,
      std::array<short,21> const& positionValueDiffs,
      ulong cycleNumber);

    BodyState(
      std::shared_ptr<BodyModel const> const& bodyModel,
      std::shared_ptr<HardwareState const> const& hardwareState,
      std::shared_ptr<BodyControl> const& bodyControl,
      ulong motionCycleNumber);

    std::shared_ptr<LimbPosition const> getLimb(std::string const& name) const;

    std::shared_ptr<JointPosition const> getJoint(JointId jointId) const;

//    void visitJoints(std::function<void(std::shared_ptr<Joint const>)> action) const;

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

    /** Transformation describing camera frame in agent frame, used to
     * transform camera coordinates to agent coordinates */
    Eigen::Affine3d const& getAgentCameraTransform() const { return d_agentCameraTr; }

    /** Transformation describing agent frame in camera frame, used to
     * transform agent coordinates to camera coordinates */
    Eigen::Affine3d const& getCameraAgentTransform() const { return d_cameraAgentTr; }

    /** Height of torso center above the ground
     *
     * Assumes the lowest foot is flat on the floor */
    double getTorsoHeight() const { return d_torsoHeight; }

    std::array<short,21> const& getPositionValueDiffs() const { return d_positionValueDiffs; }

    Eigen::Affine3d determineFootAgentTr(bool leftFoot) const;

  private:
    /// Initialise with the specified angles (radians), and position errors (values)
    /// Indexed by JointId (i.e. 0 is ignored.), including camera tilt angle
    void initialise(std::shared_ptr<BodyModel const> const& bodyModel, std::array<double,23> const& angles);

    std::array<short,21> d_positionValueDiffs;

    double d_torsoHeight;

    std::shared_ptr<LimbPosition const> d_torso;

    std::array<std::shared_ptr<JointPosition const>,23> d_jointById;
    std::map<std::string,std::shared_ptr<LimbPosition const>> d_limbByName;

    Eigen::Affine3d d_cameraAgentTr;
    Eigen::Affine3d d_agentCameraTr;

    ulong d_motionCycleNumber;

  public:
    // Needed when having fixed sized Eigen member
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  };

  inline std::shared_ptr<LimbPosition const> BodyState::getLimb(std::string const& name) const
  {
    // NOTE cannot use '[]' on a const map
    auto const& i = d_limbByName.find(name);
    if (i == d_limbByName.end())
    {
      log::error("BodyState::getJoint") << "Invalid limb name: " << name;
      throw std::runtime_error("Invalid limb name: " + name);
    }
    return i->second;
  }

  inline std::shared_ptr<JointPosition const> BodyState::getJoint(JointId jointId) const
  {
    ASSERT(jointId >= JointId::MIN && jointId <= JointId::MAX);
    return d_jointById[(uchar)jointId];
  }

//  inline void BodyState::visitJoints(std::function<void(std::shared_ptr<Joint const>)> action) const
//  {
//    for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
//      action(d_jointById[(uchar)jointId]);
//  }
}
