#pragma once

#include <Eigen/Core>
#include <memory>
#include <opencv2/opencv.hpp>
#include <sigc++/signal.h>

#include "../MX28Alarm/mx28alarm.hh"
#include "../Configurable/configurable.hh"

class Joystick;

namespace bold
{
  class ActionModule;
  class Ambulator;
  class BodyState;
  class Camera;
  class CameraModel;
  class CM730;
  class CM730Linux;
  class DataStreamer;
  class Debugger;
  class FallDetector;
  class FieldMap;
  class GyroCalibrator;
  class HeadModule;
  class GameStateReceiver;
  class Localiser;
  class MotionLoop;
  class MotionTaskScheduler;
  class OptionTree;
  class Spatialiser;
  class VisualCortex;
  class WalkModule;

  class Agent : public Configurable
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

    std::shared_ptr<HeadModule> getHeadModule() const { return d_headModule; }
    std::shared_ptr<WalkModule> getWalkModule() const { return d_walkModule; }
    std::shared_ptr<ActionModule> getActionModule() const { return d_actionModule; }

    unsigned getTeamNumber() const { return d_teamNumber; }
    void setTeamNumber(unsigned teamNumber) { d_teamNumber = teamNumber; }
    unsigned getUniformNumber() const { return d_uniformNumber; }
    void setUniformNumber(unsigned uniformNumber) { d_uniformNumber = uniformNumber; }

    void setOptionTree(std::unique_ptr<OptionTree> tree);

    Agent(Agent const&) = delete;
    Agent& operator=(Agent const&) = delete;

    void run();
    void stop();

    sigc::signal<void> onThinkEnd;

  private:
    /// Whether we have connected to a CM730 subcontroller.
    bool d_haveBody;
    bool d_isRunning;

    std::string d_motionFile;
    unsigned d_teamNumber;
    unsigned d_uniformNumber;
    bool d_isRecordingFrames;
    bool d_autoGetUpFromFallen;
    bool d_useOptionTree;

    // Motion

    std::shared_ptr<CM730Linux> d_cm730Linux;
    std::shared_ptr<CM730> d_cm730;
    std::shared_ptr<MotionLoop> d_motionLoop;
    std::shared_ptr<MotionTaskScheduler> d_motionSchedule;
    std::shared_ptr<WalkModule> d_walkModule;
    std::shared_ptr<HeadModule> d_headModule;
    std::shared_ptr<ActionModule> d_actionModule;

    // State observers

    std::shared_ptr<FallDetector> d_fallDetector;
    std::shared_ptr<GyroCalibrator> d_gyroCalibrator;

    // Components

    std::shared_ptr<Ambulator> d_ambulator;
    std::shared_ptr<Camera> d_camera;
    std::shared_ptr<CameraModel> d_cameraModel;
    std::shared_ptr<DataStreamer> d_streamer;
    std::shared_ptr<Debugger> d_debugger;
    std::shared_ptr<FieldMap> d_fieldMap;
    std::shared_ptr<Joystick> d_joystick;
    std::shared_ptr<Spatialiser> d_spatialiser;
    std::shared_ptr<Localiser> d_localiser;
    std::shared_ptr<VisualCortex> d_visualCortex;
    std::shared_ptr<GameStateReceiver> d_gameStateReceiver;

    double d_joystickXAmpMax;
    double d_joystickYAmpMax;
    double d_joystickAAmpMax;

    std::unique_ptr<OptionTree> d_optionTree;

    void initCamera();

    void registerStateTypes();

    void think();

    void readSubBoardData();

    void standUpIfFallen();

    void processInputCommands();

    void readStaticHardwareState();
  };
}
