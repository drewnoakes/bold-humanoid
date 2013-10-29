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
    
    std::vector<std::pair<PyObject*,PyObject*> > getObservedLineSegments() const
    {
      std::vector<std::pair<PyObject*,PyObject*> > out;
      std::vector<bold::LineSegment3d> in = $self->getObservedLineSegments();
      for (int i = 0; i < in.size(); ++i)
      {
        PyObject *resultobj1 = 0;
        PyObject *resultobj2 = 0;
        Eigen::Vector3d p1 = in[i].p1();
        Eigen::Vector3d p2 = in[i].p2();
        ConvertFromEigenToNumPyMatrix(&resultobj1, &p1);
        ConvertFromEigenToNumPyMatrix(&resultobj2, &p2);
        out.push_back(std::pair<PyObject*,PyObject*>(resultobj1, resultobj2));
      }
      return out;
    }

  };
}
