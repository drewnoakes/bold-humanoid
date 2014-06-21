#pragma once

#include "../bodymodel.hh"

#include <map>

namespace bold
{
  class DarwinBodyModel : public BodyModel
  {
  public:
    DarwinBodyModel();

    std::shared_ptr<Limb const> const& getRoot() const override { return d_torso; }
    std::shared_ptr<Joint const> const& getJoint(JointId jointId) const override;
    std::shared_ptr<Joint const> const& getJoint(std::string name) const override;
    std::shared_ptr<Limb const> const& getLimb(std::string name) const override;

    void visitLimbs(std::function<void(std::shared_ptr<Limb const> const&)> visitor) const
    {
      for (auto const& pair : d_limbByName)
        visitor(pair.second);
    }

  private:
    std::shared_ptr<Limb> createLimb(std::string name);
    std::shared_ptr<Joint> createJoint(JointId jointId, std::string name);

    std::shared_ptr<Limb const> d_torso;
    std::array<std::shared_ptr<Joint const>,23> d_jointById;
    std::map<std::string,std::shared_ptr<Joint const>> d_jointByName;
    std::map<std::string,std::shared_ptr<Limb const>> d_limbByName;
  };
}
