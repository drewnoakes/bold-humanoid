#ifndef BOLD_AGENT_MODEL_HH
#define BOLD_AGENT_MODEL_HH

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

#include "../stateobject.hh"
#include "../../BodyPart/bodypart.hh"
#include "../../robotis/Framework/include/JointData.h"
#include "../../util/Lazy.hh"

namespace bold
{
  class BodyState : public StateObject
  {
  public:
    BodyState(double angles[])
    : d_torso(),
      d_jointById(),
      d_limbByName()
    {
      initBody(angles);
      updatePosture();
      d_torsoHeight = Lazy<double>([this]()
      {
        return std::make_shared<double>(
          std::max(
            this->getLimb("lFoot")->transform.inverse().translation().z(),
            this->getLimb("rFoot")->transform.inverse().translation().z()
          )
        );
      });

      Eigen::Affine3d const& cameraTransform = getLimb("camera")->transform;
      Eigen::Affine3d const& footTransform = getLimb("rFoot")->transform;
      d_cameraGroundTransform = footTransform.inverse() * cameraTransform;
    };

    void updatePosture();

    std::shared_ptr<Limb const> getTorso() const
    {
      return d_torso;
    }

    std::shared_ptr<Limb const> getLimb(std::string const& name) const
    {
      // NOTE cannot use '[]' on a const map
      auto const& i = d_limbByName.find(name);
      if (i == d_limbByName.end())
        throw std::runtime_error("Invalid limb name: " + name);
      return i->second;
    }

    std::shared_ptr<Joint const> getJoint(unsigned jointId) const
    {
      assert(jointId > 0 && jointId < Robot::JointData::NUMBER_OF_JOINTS);

      // NOTE cannot use '[]' on a const map
      auto const& i = d_jointById.find(jointId);
      if (i == d_jointById.end())
        throw std::runtime_error("Invalid JointId" /*+ jointId*/);
      return i->second;
    }

    void visitJoints(std::function<void(std::shared_ptr<Joint const>)> action)
    {
      for (unsigned jointId = 1; jointId < Robot::JointData::NUMBER_OF_JOINTS; jointId++)
      {
        auto joint = getJoint(jointId);
        action(joint);
      }
    }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

    Eigen::Affine3d getCameraToGroundTransform() const { return d_cameraGroundTransform; }

    double getTorsoHeight() const
    {
      return *d_torsoHeight.value();
    }

  private:
    void initBody(double angles[]);

    Lazy<double> d_torsoHeight;
    std::shared_ptr<Limb> d_torso;
    std::map<unsigned, std::shared_ptr<Joint>> d_jointById;
    std::map<std::string, std::shared_ptr<Limb>> d_limbByName;
    Eigen::Affine3d d_cameraGroundTransform;
  };
}

#endif
