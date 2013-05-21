#pragma once

#include <string.h>

#include "../motionmodule.hh"

namespace bold
{
  class WalkModule : public MotionModule
  {
  public:
    enum
    {
      PHASE0 = 0,
      PHASE1 = 1,
      PHASE2 = 2,
      PHASE3 = 3
    };

  private:
    static constexpr double EYE_TILT_OFFSET_ANGLE = 40.0; // degrees
    static constexpr double THIGH_LENGTH = 93.0; // mm
    static constexpr double CALF_LENGTH = 93.0; // mm
    static constexpr double ANKLE_LENGTH = 33.5; // mm
    static constexpr double LEG_LENGTH = THIGH_LENGTH + CALF_LENGTH + ANKLE_LENGTH;

    double d_periodTime;
    /// Double support phase ratio.
    double d_dspRatio;
    /// Single support phase ratio.
    double d_sspRatio;
    double d_xSwapPeriodTime;
    double d_xMovePeriodTime;
    double d_ySwapPeriodTime;
    double d_yMovePeriodTime;
    double d_zSwapPeriodTime;
    double d_zMovePeriodTime;
    double d_aMovePeriodTime;
    double d_sspTime;
    double d_sspTimeStartL;
    double d_sspTimeEndL;
    double d_sspTimeStartR;
    double d_sspTimeEndR;
    double d_phaseTime1;
    double d_phaseTime2;
    double d_phaseTime3;

    double d_xOffset;
    double d_yOffset;
    double d_zOffset;
    double d_rOffset;
    double d_pOffset;
    double d_aOffset;

    double d_xSwapPhaseShift;
    double d_xSwapAmplitude;
    double d_xSwapAmplitudeaShift;
    double d_xMovePhaseShift;
    double d_xMoveAmplitude;
    double d_xMoveAmplitudeShift;
    double d_ySwapPhaseShift;
    double d_ySwapAmplitude;
    double d_ySwapAmplitudeShift;
    double d_yMovePhaseShift;
    double d_yMoveAmplitude;
    double d_yMoveAmplitudeShift;
    double d_zSwapPhaseShift;
    double d_zSwapAmplitude;
    double d_zSwapAmplitudeShift;
    double d_zMovePhaseShift;
    double d_zMoveAmplitude;
    double d_zMoveAmplitudeShift;
    double d_aMovePhaseShift;
    double d_aMoveAmplitude;
    double d_aMoveAmplitudeShift;

    double d_pelvisOffset;
    double d_pelvisSwing;
    double d_hipPitchOffset;
    double d_armSwingGain;

    bool d_isStopRequested;
    bool d_isRunning;
    double d_time;

    int    d_phase;
    double d_bodySwingY;
    double d_bodySwingZ;

    int d_outValue[14];

    static double wsin(double time, double period, double period_shift, double mag, double mag_shift);
    static bool computeIK(double *out, double x, double y, double z, double a, double b, double c);

    void updateTimeParams();
    void updateMovementParams();
    void updateBalanceParams();

  public:
    WalkModule();

    virtual ~WalkModule();

    // WalkModule initial pose
    double X_OFFSET;
    double Y_OFFSET;
    double Z_OFFSET;
    double A_OFFSET;
    double P_OFFSET;
    double R_OFFSET;

    // WalkModule control
    double PERIOD_TIME;
    double DSP_RATIO;
    double STEP_FB_RATIO;
    double X_MOVE_AMPLITUDE;
    double Y_MOVE_AMPLITUDE;
    double Z_MOVE_AMPLITUDE;
    double A_MOVE_AMPLITUDE;
    bool A_MOVE_AIM_ON;

    // Balance control
    bool   BALANCE_ENABLE;
    double BALANCE_KNEE_GAIN;
    double BALANCE_ANKLE_PITCH_GAIN;
    double BALANCE_HIP_ROLL_GAIN;
    double BALANCE_ANKLE_ROLL_GAIN;
    double Y_SWAP_AMPLITUDE;
    double Z_SWAP_AMPLITUDE;
    double ARM_SWING_GAIN;
    double PELVIS_OFFSET;
    double HIP_PITCH_OFFSET;

    int    P_GAIN;
    int    I_GAIN;
    int    D_GAIN;

    void initialize() override;
    void step(JointSelection const& selectedJoints) override;
    void applyHead(std::shared_ptr<HeadSection> head) override;
    void applyArms(std::shared_ptr<ArmSection> arms) override;
    void applyLegs(std::shared_ptr<LegSection> legs) override;

    int getCurrentPhase() const  { return d_phase; }
    double getBodySwingY() const { return d_bodySwingY; }
    double getBodySwingZ() const { return d_bodySwingZ; }

    void start();
    void stop();
    bool isRunning();
  };
}
