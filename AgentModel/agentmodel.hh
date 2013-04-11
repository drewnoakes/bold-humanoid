#ifndef BOLD_AGENT_MODEL_HH
#define BOLD_AGENT_MODEL_HH

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

#include "../robotis/Framework/include/JointData.h"
#include "../BodyPart/bodypart.hh"

namespace bold
{
  // TODO rename as BodyModelState, and make subclass of StateObject

  class AgentModel
  {
  public:
    AgentModel()
    {
      initBody();
    };

    void updatePosture();

    std::shared_ptr<Limb const> getTorso() const { return d_torso; }

    std::shared_ptr<Limb const> getLimb(std::string const& name) const
    {
      // NOTE cannot use '[]' on a const map
      auto const& i = d_limbByName.find(name);
      if (i == d_limbByName.end())
        throw std::runtime_error("Invalid limb name: " + name);
      return i->second;
    }

    std::shared_ptr<Joint> getJoint(int jointId) const
    {
      assert(jointId > 0 && jointId < Robot::JointData::NUMBER_OF_JOINTS);

      // NOTE cannot use '[]' on a const map
      auto const& i = d_jointById.find(jointId);
      if (i == d_jointById.end())
        throw std::runtime_error("Invalid JointId: " + jointId);
      return i->second;
    }

    void visitJoints(std::function<void(Joint&)> action)
    {
      for (int jointId = Robot::JointData::ID_R_SHOULDER_PITCH; jointId < Robot::JointData::NUMBER_OF_JOINTS; jointId++)
      {
        Joint& joint = *getJoint(jointId);
        action(joint);
      }
    }

  private:
    void initBody();

    std::map<int, std::shared_ptr<Joint>> d_jointById;

    std::shared_ptr<Limb> d_torso;
    std::map<std::string, std::shared_ptr<Limb>> d_limbByName;
  };
}

#endif
