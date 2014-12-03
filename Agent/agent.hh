#pragma once

#include <memory>

#include "../Clock/clock.hh"
#include "../util/loop.hh"

namespace bold
{
  class BehaviourControl;
  class BodyState;
  class ButtonObserver;
  class Camera;
  class CameraModel;
  class DataStreamer;
  class Debugger;
  class DrawBridgeComms;
  class FallDetector;
  class GameStateReceiver;
  class GyroCalibrator;
  class HealthAndSafety;
  class HeadModule;
  class JamDetector;
  class Localiser;
  class MessageCounter;
  class MotionLoop;
  class MotionScriptModule;
  class MotionTaskScheduler;
  class Odometer;
  class OptionTree;
  class OpenTeamCommunicator;
  class OrientationTracker;
  class RemoteControl;
  class RoleDecider;
  class Spatialiser;
  class SuicidePill;
  class VisualCortex;
  class Vocaliser;
  class Voice;
  class WalkModule;

  typedef unsigned char uchar;
  typedef unsigned long ulong;

  enum class TeamColour;

  class Agent : public Loop
  {
  public:
    static void registerStateTypes();

    Agent();

    std::shared_ptr<ButtonObserver> getButtonObserver() const { return d_buttonObserver; }
    std::shared_ptr<Camera> getCamera() const { return d_camera; }
    std::shared_ptr<CameraModel> getCameraModel() const { return d_cameraModel; }
    std::shared_ptr<BehaviourControl> getBehaviourControl() const { return d_behaviourControl; }

    std::shared_ptr<HeadModule> getHeadModule() const { return d_headModule; }
    std::shared_ptr<WalkModule> getWalkModule() const { return d_walkModule; }
    std::shared_ptr<MotionScriptModule> getMotionScriptModule() const { return d_motionScriptModule; }

    std::shared_ptr<FallDetector const> getFallDetector() const { return d_fallDetector; }
    std::shared_ptr<Voice> getVoice() const { return d_voice; }

    void setOptionTree(std::shared_ptr<OptionTree> tree);

    Agent(Agent const&) = delete;
    Agent& operator=(Agent const&) = delete;

    void run();
    void requestStop();

    bool isShutdownRequested() const { return d_isShutdownRequested; }

    ulong getThinkCycleNumber() const;

    double getUptimeSeconds() const { return Clock::getSecondsSince(d_startTime); }

  private:
    void onLoopStart() override;
    void onStep(ulong cycleNumber) override;
    void onStopped() override;

    bool d_isShutdownRequested;

    uchar const d_teamNumber;
    uchar const d_uniformNumber;
    TeamColour const d_teamColour;

    // Motion

    std::shared_ptr<MotionLoop> d_motionLoop;
    std::shared_ptr<MotionTaskScheduler> d_motionSchedule;
    std::shared_ptr<WalkModule> d_walkModule;
    std::shared_ptr<HeadModule> d_headModule;
    std::shared_ptr<MotionScriptModule> d_motionScriptModule;

    // State observers

    std::shared_ptr<Vocaliser> d_vocaliser;
    std::shared_ptr<ButtonObserver> d_buttonObserver;
    std::shared_ptr<FallDetector const> d_fallDetector;
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

    std::shared_ptr<Camera> d_camera;
    std::shared_ptr<CameraModel> d_cameraModel;
    std::shared_ptr<DataStreamer> d_streamer;
    std::shared_ptr<Debugger> d_debugger;
    std::shared_ptr<DrawBridgeComms> d_drawBridgeComms;
    std::shared_ptr<GameStateReceiver> d_gameStateReceiver;
    std::shared_ptr<Localiser> d_localiser;
    std::shared_ptr<MessageCounter> d_messageCounter;
    std::shared_ptr<OptionTree> d_optionTree;
    std::shared_ptr<RemoteControl> d_remoteControl;
    std::shared_ptr<Spatialiser> d_spatialiser;
    std::shared_ptr<VisualCortex> d_visualCortex;
    std::shared_ptr<Voice> d_voice;

    ulong d_cycleNumber;

    Clock::Timestamp d_startTime;
  };
}
