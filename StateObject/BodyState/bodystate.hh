#pragma once

#include <cassert>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

#include "../stateobject.hh"
#include "../../BodyPart/bodypart.hh"
#include "../../JointId/jointid.hh"
#include "../../util/log.hh"

namespace bold
{
  typedef unsigned char uchar;

  class BodyControl;
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
    static std::shared_ptr<BodyState const> zero(ulong thinkCycleNumber = 0);

    static Eigen::Vector3d distanceBetween(std::shared_ptr<BodyPart const> const& p1, std::shared_ptr<BodyPart const> const& p2)
    {
      return p2->getPosition() - p1->getPosition();
    }

    /// Initialise with the specified angles, in radians. Indexed by JointId (i.e. 0 is ignored.)
    BodyState(double angles[23],
              std::vector<int> positionValueDiffs,
              ulong motionCycleNumber);
    BodyState(std::shared_ptr<HardwareState const> const& hardwareState,
              std::shared_ptr<BodyControl> const& bodyControl,
              ulong motionCycleNumber);

    std::shared_ptr<Limb const> getTorso() const { return d_torso; }

    std::shared_ptr<Limb const> getLimb(std::string const& name) const;

    std::shared_ptr<Joint const> getJoint(JointId jointId) const;

    void visitJoints(std::function<void(std::shared_ptr<Joint const>)> action) const;
    
    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

    Eigen::Affine3d const& getCameraAgentTransform() const { return d_cameraAgentTransform; }
    Eigen::Affine3d const& getAgentCameraTransform() const { return d_agentCameraTransform; }

    double getTorsoHeight() const { return d_torsoHeight; }

    std::vector<int> const getPositionValueDiffs() const { return d_positionValueDiffs; }

  private:
    /// Initialise with the specified angles (radians), and position errors (values)
    /// Indexed by JointId (i.e. 0 is ignored.), including camera tilt angle
    void initialise(double angles[23]);

    std::vector<int> d_positionValueDiffs;

    double d_torsoHeight;
    std::shared_ptr<Limb> d_torso;
    std::shared_ptr<Joint> d_jointById[23];
    std::map<std::string, std::shared_ptr<Limb>> d_limbByName;

    Eigen::Affine3d d_cameraAgentTransform;
    Eigen::Affine3d d_agentCameraTransform;

    ulong d_motionCycleNumber;

  public:
    // Needed when having fixed sized Eigen member
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  };

  inline std::shared_ptr<Limb const> BodyState::getLimb(std::string const& name) const
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

  inline std::shared_ptr<Joint const> BodyState::getJoint(JointId jointId) const
  {
    assert(jointId >= JointId::MIN && jointId <= JointId::MAX);
    return d_jointById[(uchar)jointId];
  }

  inline void BodyState::visitJoints(std::function<void(std::shared_ptr<Joint const>)> action) const
  {
    for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      action(d_jointById[(uchar)jointId]);
  }
}
