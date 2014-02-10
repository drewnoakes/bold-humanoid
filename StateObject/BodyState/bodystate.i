%{
#include <StateObject/BodyState/bodystate.hh>
%}

namespace bold
{

  class BodyState : public StateObject
  {
  public:
    double getTorsoHeight() const;

    std::shared_ptr<Limb const> getTorso() const;
    std::shared_ptr<Limb const> getLimb(std::string const& name) const;
    std::shared_ptr<Joint const> getJoint(JointId jointId) const;
  };

}
