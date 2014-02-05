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

namespace bold
{
  typedef unsigned char uchar;

  class BodyControl;
  class HardwareState;
  template<typename> class Setting;;

  class BodyState : public StateObject
  {
  public:
    static std::shared_ptr<BodyState const> zero(ulong thinkCycleNumber = 0);

    /// Initialise with the specified angles, in radians. Indexed by JointId (i.e. 0 is ignored.)
    BodyState(double angles[], std::vector<int> positionValueDiffs, ulong motionCycleNumber);
    BodyState(std::shared_ptr<HardwareState const> const& hardwareState, std::shared_ptr<BodyControl> const& bodyControl, ulong motionCycleNumber);

    std::shared_ptr<Limb const> getTorso() const { return d_torso; }

    std::shared_ptr<Joint const> getHeadPanJoint() const { return getJoint(JointId::HEAD_PAN); }

    std::shared_ptr<Limb const> getLimb(std::string const& name) const
    {
      // NOTE cannot use '[]' on a const map
      auto const& i = d_limbByName.find(name);
      if (i == d_limbByName.end())
        throw std::runtime_error("Invalid limb name: " + name);
      return i->second;
    }

    std::shared_ptr<Joint const> getJoint(JointId jointId) const
    {
      assert(jointId >= JointId::MIN && jointId <= JointId::MAX);

      // NOTE cannot use '[]' on a const map
      auto const& i = d_jointById.find((uchar)jointId);
      if (i == d_jointById.end())
        throw std::runtime_error("Invalid JointId" /*+ jointId*/);
      return i->second;
    }

    void visitJoints(std::function<void(std::shared_ptr<Joint const>)> action)
    {
      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      {
        auto joint = getJoint((JointId)jointId);
        action(joint);
      }
    }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

    Eigen::Affine3d const& getCameraAgentTransform() const { return d_cameraAgentTransform; }

    double getTorsoHeight() const { return d_torsoHeight; }

    std::vector<int> const getPositionValueDiffs() const { return d_positionValueDiffs; }

  private:
    /// Initialise with the specified angles (radians), and position errors (values)
    /// Indexed by JointId (i.e. 0 is ignored.)
    void initialise(double angles[]);

    std::vector<int> d_positionValueDiffs;

    double d_torsoHeight;
    std::shared_ptr<Limb> d_torso;
    // TODO replace with a vector<sp<Joint>> for performance of lookup
    std::map<uchar, std::shared_ptr<Joint>> d_jointById;
    std::map<std::string, std::shared_ptr<Limb>> d_limbByName;

    /// Transform of camera, including rotation of torso in agent/world frames
    Eigen::Affine3d d_cameraAgentTransform;

    ulong d_motionCycleNumber;

    Setting<double>* d_cameraTilt;

  public:
    // Needed when having fixed sized Eigen member
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  };
}
