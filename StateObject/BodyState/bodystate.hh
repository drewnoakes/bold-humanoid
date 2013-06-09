#pragma once

#include <functional>
#include <future>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

#include "../stateobject.hh"
#include "../../BodyPart/bodypart.hh"
#include "../JointId/jointid.hh"

namespace bold
{
  typedef unsigned char uchar;
  
  class BodyState : public StateObject
  {
  public:
    BodyState(double angles[]);

    void updatePosture();

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

    Eigen::Affine3d getCameraAgentTransform() const { return d_cameraAgentTransform; }

    double getTorsoHeight() const { return d_torsoHeight; }

  private:
    void initBody(double angles[]);

    double d_torsoHeight;
    std::shared_ptr<Limb> d_torso;
    // TODO replace with a vector<sp<Joint>> for performance of lookup
    std::map<uchar, std::shared_ptr<Joint>> d_jointById;
    std::map<std::string, std::shared_ptr<Limb>> d_limbByName;

    /// Rotation of the torso, wrt the agent/world frames
    Eigen::Affine3d d_torsoWorldRotation;
    /// Transform of camera, including rotation of torso in agent/world frames
    Eigen::Affine3d d_cameraAgentTransform;

  public:
    // Needed when having fixed sized Eigen member
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  };
}
