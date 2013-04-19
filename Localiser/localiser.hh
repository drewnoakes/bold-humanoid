#ifndef BOLD_LOCALISER_HH
#define BOLD_LOCALISER_HH

#include <memory>
#include <functional>

#include "../AgentPosition/agentposition.hh"

namespace bold
{
  template<int DIM>
  class ParticleFilter;
  class FieldMap;

  class Localiser
  {
  public:
    Localiser(std::shared_ptr<FieldMap> fieldMap);

    void predict(Eigen::Affine3d motion);

    void update();

    AgentPosition position() const { return d_pos; }

  private:
    void updateStateObject();

    AgentPosition d_pos;
    std::shared_ptr<FieldMap> d_fieldMap;
    std::shared_ptr<ParticleFilter<3>> d_filter;
    std::function<double()> d_fieldXRng;
    std::function<double()> d_fieldYRng;
    std::function<double()> d_thetaRng;
  };
}

#endif