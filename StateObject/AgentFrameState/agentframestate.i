%{
#include <StateObject/AgentFrameState/agentframestate.hh>
%}

namespace bold
{

  class AgentFrameState : public StateObject
  {
  public:
    bool isBallVisible() const;
  };
  
  %extend AgentFrameState {
  public:
    // SWIG has issues with Maybe
    std::shared_ptr<Eigen::Vector3d> getBallObservation() const
    {
      return ($self->getBallObservation());
    }

    // 
    std::vector<PyObject*> getGoalObservations() const
    {
      std::vector<PyObject*> out;
      std::vector<Eigen::Vector3d> in = $self->getGoalObservations();
      for (int i = 0; i < in.size(); ++i)
      {
        PyObject *resultobj = 0;
        ConvertFromEigenToNumPyMatrix<Eigen::Vector3d>(&resultobj, &(in[i]));
        out.push_back(resultobj);
      }
      return out;
    }
    

  };
}
