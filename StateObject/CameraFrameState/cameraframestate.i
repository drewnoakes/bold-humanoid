%{
#include <StateObject/CameraFrameState/cameraframestate.hh>
%}

namespace bold
{

  class CameraFrameState : public StateObject
  {
  public:
    bool isBallVisible() const;
  };
  
  %extend CameraFrameState {
  public:
    // SWIG has issues with Maybe
    std::shared_ptr<Eigen::Vector2d> getBallObservation() const
    {
      return ($self->getBallObservation());
    }
    // 
    std::vector<PyObject*> getGoalObservations() const
    {
      std::vector<PyObject*> out;
      std::vector<Eigen::Vector2d> in = $self->getGoalObservations();
      for (int i = 0; i < in.size(); ++i)
      {
        PyObject *resultobj = 0;
        ConvertFromEigenToNumPyMatrix(&resultobj, &(in[i]));
        out.push_back(resultobj);
      }
      return out;
    }
    
    std::vector<std::pair<PyObject*,PyObject*> > getObservedLineSegments() const
    {
      std::vector<std::pair<PyObject*,PyObject*> > out;
      std::vector<bold::LineSegment2i> in = $self->getObservedLineSegments();
      for (int i = 0; i < in.size(); ++i)
      {
        PyObject *resultobj1 = 0;
        PyObject *resultobj2 = 0;
        Eigen::Vector2i p1 = in[i].p1();
        Eigen::Vector2i p2 = in[i].p2();
        ConvertFromEigenToNumPyMatrix(&resultobj1, &p1);
        ConvertFromEigenToNumPyMatrix(&resultobj2, &p2);
        out.push_back(std::pair<PyObject*,PyObject*>(resultobj1, resultobj2));
      }
      return out;
    }
  };

}
