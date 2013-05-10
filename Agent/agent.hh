#pragma once

#include <Eigen/Core>
#include <memory>
#include <opencv2/opencv.hpp>
#include <sigc++/signal.h>

#include "../MX28Alarm/mx28alarm.hh"
#include "../minIni/minIni.h"

class Joystick;

namespace bold
{
  class Action;
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
  class Head;
  class GameStateReceiver;
  class Localiser;
  class MotionLoop;
  class OptionTree;
  class Spatialiser;
  class VisualCortex;
  class Walking;

  enum class ActionPage
  {
    ForwardGetUp = 10,
    BackwardGetUp = 11,
    KickRight = 12,
    KickLeft = 13
  };

  class Agent
  {
  public:
    Agent(std::string const& U2D_dev,
          std::string const& confFile,
          std::string const& motionFile,
          unsigned teamNumber,
          unsigned uniformNumber,
          bool useJoystick,
          bool autoGetUpFromFallen,
          bool useOptionTree,
          bool recordFrames,
          bool ignoreGameController);

    Agent(Agent const&) = delete;
    Agent& operator=(Agent const&) = delete;

    void run();
    void stop();

    sigc::signal<void> onThinkEnd;

  private:
    /// Whether we have connected to a CM730 subcontroller.
    bool d_haveBody;
    bool d_isRunning;
    minIni d_ini;
    std::string d_motionFile;
    unsigned d_teamNumber;
    unsigned d_uniformNumber;
    bool d_isRecordingFrames;
    bool d_autoGetUpFromFallen;
    bool d_useOptionTree;
    bool d_ignoreGameController;

    // Motion

    std::shared_ptr<CM730Linux> d_cm730Linux;
    std::shared_ptr<CM730> d_cm730;
    std::shared_ptr<MotionLoop> d_motionLoop;
    std::shared_ptr<Walking> d_walkModule;
    std::shared_ptr<Head> d_headModule;
    std::shared_ptr<Action> d_actionModule;

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

    void initCamera(minIni const& ini);

    void registerStateTypes();

    void think();

    void readSubBoardData();

    void standUpIfFallen();

    void processInputCommands();
  };
}
