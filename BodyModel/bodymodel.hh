#pragma once

#include "../JointId/jointid.hh"
#include "../BodyPart/bodypart.hh"

namespace bold
{
  class BodyModel
  {
  public:
    virtual std::shared_ptr<Limb const> const& getRoot() const = 0;
    virtual std::shared_ptr<Joint const> const& getJoint(JointId jointId) const = 0;
    virtual std::shared_ptr<Joint const> const& getJoint(std::string name) const = 0;
    virtual std::shared_ptr<Limb const> const& getLimb(std::string name) const = 0;
  };
}
