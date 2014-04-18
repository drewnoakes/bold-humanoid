#pragma once

#include <Eigen/Core>
#include <memory>

#include "../motionmodule.hh"
#include "../../Smoother/LinearSmoother/linearsmoother.hh"

namespace bold
{
  class WalkEngine;
  template<typename> class Setting;

  enum class WalkStatus
  {
    Stopped,
    Starting,
    Walking,
    Stabilising
  };

  std::string getWalkStatusName(WalkStatus status);

  class WalkModule : public MotionModule
  {
  public:
    WalkModule(std::shared_ptr<MotionTaskScheduler> scheduler);
    ~WalkModule() override = default;

    void initialize() override {};

    void step(std::shared_ptr<JointSelection> selectedJoints) override;

    void applyHead(HeadSection* head) override;
    void applyArms(ArmSection* arms) override;
    void applyLegs(LegSection* legs) override;

    /** Slow walking to a stop and stabilise.
     */
    void stop();

    /** Stop walking abruptly. Likely unstable.
     *
     * Call stop instead to have walking stop smoothly.
     */
    void stopImmediately();

    bool isRunning() const { return d_status != WalkStatus::Stopped; }

    WalkStatus getStatus() const { return d_status; }

    /**
     * Set the direction of motion, where positive X is in the forwards
     * direction, and positive Y is to the left. The length of the vector
     * determines the velocity of motion (unspecified units).
     *
     * Note that the same value will result in different speeds in the X and Y
     * axes.
     */
    void setMoveDir(Eigen::Vector2d const& moveDir);

    /**
     * Set the rate of turning, where positive values turn right (clockwise)
     * and negative values turn left (counter-clockwise) (unspecified units).
     */
    void setTurnAngle(double turnSpeed);

  private:
    WalkModule(const WalkModule&) = delete;
    WalkModule& operator=(const WalkModule&) = delete;

    std::shared_ptr<WalkEngine> d_walkEngine;

    Setting<int>* d_stabilisationTimeMillis;
    int d_stabilisationCycleCount;
    int d_stabilisationCyclesRemaining;

    LinearSmoother d_xAmp;
    LinearSmoother d_yAmp;
    LinearSmoother d_turnAmp;

    Setting<double>* d_maxHipPitchAtSpeed;
    Setting<double>* d_minHipPitch;
    Setting<double>* d_maxHipPitch;
    Setting<double>* d_fwdAccelerationHipPitchFactor;

    bool d_turnAngleSet;
    bool d_moveDirSet;
    WalkStatus d_status;
  };
}
