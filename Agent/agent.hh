#pragma once

#include <Eigen/Core>
#include <minIni.h>
#include <memory>
#include <opencv2/opencv.hpp>
#include <sigc++/signal.h>

#include "../MX28Alarm/mx28alarm.hh"

class Joystick;

namespace robotis
{
  class CM730;
  class LinuxCM730;
  class LinuxMotionTimer;
}

namespace bold
{
  class Ambulator;
  class BodyState;
  class Camera;
  class CameraModel;
  class DataStreamer;
  class Debugger;
  class FieldMap;
  class GameStateReceiver;
  class Localiser;
  class OptionTree;
  class Spatialiser;
  class VisualCortex;

  class Agent
  {
  public:
    Agent(std::string const& U2D_dev,
          minIni const& ini,
          std::string const& motionFile,
          unsigned teamNumber,
          unsigned uniformNumber,
          bool useJoystick,
          bool autoGetUpFromFallen,
          bool useOptionTree,
          bool recordFrames,
          bool ignoreGameController);

    
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

    void setOptionTree(std::unique_ptr<OptionTree>& tree)
    {
      d_optionTree = std::move(tree);
    }

    void run();
    void stop();

    sigc::signal<void> onThinkEnd;

  private:
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

    std::shared_ptr<robotis::LinuxCM730> d_linuxCM730;
    std::shared_ptr<robotis::CM730> d_CM730;
    std::shared_ptr<robotis::LinuxMotionTimer> d_motionTimer;

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

    bool initMotionManager(minIni const& ini);

    void registerStateTypes();

    void think();

    void readSubBoardData();

    void standUpIfFallen();

    void processInputCommands();
  };
}
