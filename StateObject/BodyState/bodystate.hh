#pragma once

#include <map>
#include <memory>
#include <vector>
#include <array>
#include <Eigen/Geometry>

#include "../stateobject.hh"
#include "../../BodyPart/bodypart.hh"
#include "../../JointId/jointid.hh"

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
    LimbPosition(std::shared_ptr<Limb const> limb, Eigen::Affine3d const& transform);

    std::shared_ptr<Limb const> const& getLimb() const { return d_limb; }

    Eigen::Vector3d getCentreOfMassPosition() const;

  private:
    std::shared_ptr<Limb const> d_limb;
  };

  class JointPosition : public BodyPartPosition
  {
  public:
    JointPosition(std::shared_ptr<Joint const> joint, Eigen::Affine3d const& transform, double angleRads);

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

    void visitJoints(std::function<void(std::shared_ptr<JointPosition const> const&)> visitor) const;
    void visitLimbs(std::function<void(std::shared_ptr<LimbPosition const> const&)> visitor) const;

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }

    /** Transformation describing camera frame in agent frame, used to
     * transform camera coordinates to agent coordinates */
    Eigen::Affine3d const& getAgentCameraTransform() const { return d_agentCameraTr; }

    /** Transformation describing agent frame in camera frame, used to
     * transform agent coordinates to camera coordinates */
    Eigen::Affine3d const& getCameraAgentTransform() const { return d_cameraAgentTr; }

    Eigen::Vector3d const& getCentreOfMass() const;

    /** Height of torso center above the ground
     *
     * Assumes the lowest foot is flat on the floor */
    double getTorsoHeight() const { return d_torsoHeight; }

    std::array<short,21> const& getPositionValueDiffById() const { return d_positionValueDiffById; }

    Eigen::Affine3d determineFootAgentTr(bool leftFoot) const;

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer>& writer) const;

    /// Initialise with the specified angles (radians), and position errors (values)
    /// Indexed by JointId (i.e. 0 is ignored.), including camera tilt angle
    void initialise(std::shared_ptr<BodyModel const> const& bodyModel, std::array<double,23> const& angles);

    std::array<short,21> d_positionValueDiffById;

    double d_torsoHeight;

    std::shared_ptr<LimbPosition const> d_torso;

    std::array<std::shared_ptr<JointPosition const>,23> d_jointById;
    std::map<std::string,std::shared_ptr<LimbPosition const>> d_limbByName;

    Eigen::Affine3d d_cameraAgentTr;
    Eigen::Affine3d d_agentCameraTr;

    mutable Eigen::Vector3d d_centreOfMass;
    mutable bool d_isCentreOfMassComputed;

    ulong d_motionCycleNumber;

  public:
    // Needed when having fixed sized Eigen member
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  };

  template<typename TBuffer>
  inline void BodyState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    writer.StartObject();
    {
      writer.String("motion-cycle");
      writer.Uint64(d_motionCycleNumber);

      writer.String("angles");
      writer.StartArray();
      {
        for (uchar j = (uchar)JointId::MIN; j <= (uchar)JointId::MAX; j++)
        {
          auto const& it = d_jointById[j];
          writer.Double(it->getAngleRads(), "%.3f");
        }
      }
      writer.EndArray();

      writer.String("errors");
      writer.StartArray();
      {
        for (uchar j = (uchar)JointId::MIN; j <= (uchar)JointId::MAX; j++)
          writer.Int(d_positionValueDiffById[j]);
      }
      writer.EndArray();

      auto com = getCentreOfMass();
      writer.String("com");
      writer.StartArray();
      writer.Double(com.x(), "%.4f");
      writer.Double(com.y(), "%.4f");
      writer.Double(com.z(), "%.4f");
      writer.EndArray();
    }
    writer.EndObject();
  }
}
