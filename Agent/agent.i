%{
#include <Agent/agent.hh>
%}

namespace bold
{
  class MotionScriptModule;
  class Ambulator;
  class BodyState;
  class Camera;
  class CameraModel;
  class DataStreamer;
  class Debugger;
  class FallDetector;
  class FieldMap;
  class GyroCalibrator;
  class HeadModule;
  class GameStateReceiver;
  class Localiser;
  class MotionLoop;
  class OpenTeamCommunicator;
  class OptionTree;
  class Spatialiser;
  class VisualCortex;
  class WalkModule;

  class Agent
  {
  public:
    Agent();

    std::shared_ptr<Ambulator> getAmbulator() const;
    std::shared_ptr<Camera> getCamera() const;
    std::shared_ptr<CameraModel> getCameraModel() const;
    std::shared_ptr<DataStreamer> getDataStreamer() const;
    std::shared_ptr<Debugger> getDebugger() const;
    std::shared_ptr<FieldMap> getFieldMap() const;
    std::shared_ptr<Joystick> getJoystick() const;
    std::shared_ptr<Spatialiser> getSpatialiser() const;
    std::shared_ptr<Localiser> getLocaliser() const;
    std::shared_ptr<VisualCortex> getVisualCortex() const;
    std::shared_ptr<GameStateReceiver> getGameStateReceiver() const;

    std::shared_ptr<HeadModule> getHeadModule() const;
    std::shared_ptr<WalkModule> getWalkModule() const;
    std::shared_ptr<MotionScriptModule> getMotionScriptModule() const;

    std::shared_ptr<FallDetector> getFallDetector() const;

    uchar getTeamNumber() const;
    uchar getUniformNumber() const;

    void run();
    void requestStop();
    void stop();
    bool isStopRequested() const;

  };

  %extend Agent {
  public:
    void setOptionTree(OptionTree* tree)
    {
      $self->setOptionTree(std::unique_ptr<bold::OptionTree>(tree));
    }

    void onThinkEndConnect(PyObject* pyFunc)
    {
      $self->onThinkEnd.connect(
        [pyFunc] {
          PyEval_CallObject(pyFunc, Py_BuildValue("()"));
        });
    }
  };
}
