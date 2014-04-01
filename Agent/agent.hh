#pragma once

#include <Eigen/Core>
#include <memory>
#include <opencv2/opencv.hpp>
#include <sigc++/signal.h>

#include "../MX28Alarm/mx28alarm.hh"
#include "../StateObject/TeamState/teamstate.hh"

class Joystick;

namespace bold
{
  class Ambulator;
  class BehaviourControl;
  class BodyState;
  class Camera;
  class CameraModel;
  class DataStreamer;
  class Debugger;
  class FallDetector;
  class FieldMap;
  class GameStateReceiver;
  class GyroCalibrator;
  class HealthAndSafety;
  class HeadModule;
  class JamDetector;
  class Localiser;
  class MotionLoop;
  class MotionScriptModule;
  class MotionTaskScheduler;
  class Odometer;
  class OptionTree;
  class OpenTeamCommunicator;
  class OrientationTracker;
  class RoleDecider;
  template<typename> class Setting;
  class Spatialiser;
  class SuicidePill;
  class VisualCortex;
  class Vocaliser;
  class Voice;
  class WalkModule;

  typedef unsigned long ulong;

  class Agent
  {
  public:
    Agent();

    std::shared_ptr<Ambulator> getAmbulator() const { return d_ambulator; }
    std::shared_ptr<Camera> getCamera() const { return d_camera; }
    std::shared_ptr<CameraModel> getCameraModel() const { return d_cameraModel; }
    std::shared_ptr<DataStreamer> getDataStreamer() const { return d_streamer; }
    std::shared_ptr<Debugger> getDebugger() const { return d_debugger; }
    std::shared_ptr<FieldMap> getFieldMap() const { return d_fieldMap; }
    std::shared_ptr<Joystick> getJoystick() const { return d_joystick; }
    std::shared_ptr<Spatialiser> getSpatialiser() const { return d_spatialiser; }
    std::shared_ptr<Localiser> getLocaliser() const { return d_localiser; }
    std::shared_ptr<VisualCortex> getVisualCortex() const { return d_visualCortex; }
    std::shared_ptr<GameStateReceiver> getGameStateReceiver() const { return d_gameStateReceiver; }
    std::shared_ptr<OpenTeamCommunicator> getOpenTeamCommunicator() const { return d_teamCommunicator; }
    std::shared_ptr<RoleDecider> getRoleDecider() const { return d_roleDecider; }
    std::shared_ptr<BehaviourControl> getBehaviourControl() const { return d_behaviourControl; }

    std::shared_ptr<HeadModule> getHeadModule() const { return d_headModule; }
    std::shared_ptr<WalkModule> getWalkModule() const { return d_walkModule; }
    std::shared_ptr<MotionScriptModule> getMotionScriptModule() const { return d_motionScriptModule; }

    std::shared_ptr<FallDetector> getFallDetector() const { return d_fallDetector; }
    std::shared_ptr<Voice> getVoice() const { return d_voice; }

    unsigned getTeamNumber() const { return d_teamNumber; }
    unsigned getUniformNumber() const { return d_uniformNumber; }

    void setOptionTree(std::shared_ptr<OptionTree> tree);

    Agent(Agent const&) = delete;
    Agent& operator=(Agent const&) = delete;

    void run();
    void requestStop();
    void stop();

    bool isStopRequested() const { return d_isStopRequested; }

    ulong getThinkCycleNumber() const { return d_cycleNumber; }

    sigc::signal<void> onThinkEnd;

  private:
    bool d_isRunning;
    bool d_isStopRequested;

    const unsigned d_teamNumber;
    const unsigned d_uniformNumber;

    // Motion

    std::shared_ptr<MotionLoop> d_motionLoop;
    std::shared_ptr<MotionTaskScheduler> d_motionSchedule;
    std::shared_ptr<WalkModule> d_walkModule;
    std::shared_ptr<HeadModule> d_headModule;
    std::shared_ptr<MotionScriptModule> d_motionScriptModule;

    // State observers

    std::shared_ptr<Vocaliser> d_vocaliser;
    std::shared_ptr<FallDetector> d_fallDetector;
    std::shared_ptr<GyroCalibrator> d_gyroCalibrator;
    std::shared_ptr<HealthAndSafety> d_healthAndSafety;
    std::shared_ptr<JamDetector> d_jamTracker;
    std::shared_ptr<OpenTeamCommunicator> d_teamCommunicator;
    std::shared_ptr<RoleDecider> d_roleDecider;
    std::shared_ptr<BehaviourControl> d_behaviourControl;
    std::shared_ptr<SuicidePill> d_suicidePill;
    std::shared_ptr<Odometer> d_odometer;
    std::shared_ptr<OrientationTracker> d_orientationTracker;

    // Components

    std::shared_ptr<Ambulator> d_ambulator;
    std::shared_ptr<Camera> d_camera;
    std::shared_ptr<CameraModel> d_cameraModel;
    std::shared_ptr<DataStreamer> d_streamer;
    std::shared_ptr<Debugger> d_debugger;
    std::shared_ptr<FieldMap> d_fieldMap;
    std::shared_ptr<GameStateReceiver> d_gameStateReceiver;
    std::shared_ptr<Joystick> d_joystick;
    std::shared_ptr<Localiser> d_localiser;
    std::shared_ptr<OptionTree> d_optionTree;
    std::shared_ptr<Spatialiser> d_spatialiser;
    std::shared_ptr<VisualCortex> d_visualCortex;
    std::shared_ptr<Voice> d_voice;

    Setting<double>* d_joystickHeadSpeed;
    Setting<double>* d_joystickXAmpMax;
    Setting<double>* d_joystickYAmpMax;
    Setting<double>* d_joystickAAmpMax;

    ulong d_cycleNumber;

    void initCamera();

    void registerStateTypes();

    void think();

    void processInputCommands();
  };
}
