%{
#include <BodyPart/bodypart.hh>
%}

namespace bold
{
  class JointId;

  struct BodyPart
  {
    std::string name;
    Eigen::Affine3d transform;

    Eigen::Vector3d getPosition();
  };

  struct Joint;

  struct Limb : public BodyPart
  {
    unsigned id;
    double weight;
    double relativeWeight;
    Eigen::Vector3d size;

    std::vector<std::shared_ptr<Joint> > joints;
  };

  struct Joint : public BodyPart
  {
    Eigen::Vector3d axis;

    JointId id;
    double angleRads;
    std::shared_ptr<BodyPart> bodyPart;
    std::pair<Eigen::Vector3d, Eigen::Vector3d> anchors;

    Eigen::Vector3d getAxisVec() const;
  };

}
