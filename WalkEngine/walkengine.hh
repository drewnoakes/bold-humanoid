#pragma once

#include <string.h>
#include <cmath>

#include "../PoseProvider/poseprovider.hh"

namespace bold
{
  template <typename> class Setting;

  /** Computes body angles for walking.
   */
  class WalkEngine : public PoseProvider
  {
  public:
    enum
    {
      /// Left foot lifting up
      PHASE0 = 0,
      /// Left foot moving down
      PHASE1 = 1,
      /// Right foot lifting up
      PHASE2 = 2,
      /// Right foot moving down
      PHASE3 = 3
    };

    /** Initialise a new WalkEngine. */
    WalkEngine();

    void reset();

    void step();

    void balance(double ratio);

    void applyHead(HeadSection* head) override;
    void applyArms(ArmSection* arms) override;
    void applyLegs(LegSection* legs) override;

    /** Whether the body would be stable if walking would cease now.
     *
     * Only true twice each period.
     */
    bool canStopNow() const;

    int    getCurrentPhase() const { return d_phase; }
    double getBodySwingY()   const { return d_bodySwingY; }
    double getBodySwingZ()   const { return d_bodySwingZ; }

    double X_MOVE_AMPLITUDE;
    double Y_MOVE_AMPLITUDE;
    double A_MOVE_AMPLITUDE;
    double HIP_PITCH_OFFSET;

  private:
    static constexpr int TIME_UNIT = 8; //msec

    static constexpr double THIGH_LENGTH = 93.0; // mm
    static constexpr double CALF_LENGTH = 93.0; // mm
    static constexpr double ANKLE_LENGTH = 33.5; // mm
    static constexpr double LEG_LENGTH = THIGH_LENGTH + CALF_LENGTH + ANKLE_LENGTH;

    static double wsin(double time, double period, double periodShift, double mag, double magShift);
    static bool computeIK(double *out, double x, double y, double z, double a, double b, double c);

    void updateTimeParams();
    void updateMovementParams();
    void updateBalanceParams();

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

    static constexpr double d_xSwapPhaseShift = M_PI;
    static constexpr double d_xSwapAmplitudeaShift = 0;
    static constexpr double d_xMovePhaseShift = M_PI / 2;
    static constexpr double d_xMoveAmplitudeShift = 0;
    static constexpr double d_ySwapPhaseShift = 0;
    static constexpr double d_ySwapAmplitudeShift = 0;
    static constexpr double d_yMovePhaseShift = M_PI / 2;
    static constexpr double d_zSwapPhaseShift = M_PI * 3 / 2;
    static constexpr double d_zMovePhaseShift = M_PI / 2;
    static constexpr double d_aMovePhaseShift = M_PI / 2;

//     double d_xSwapPhaseShift;
    double d_xSwapAmplitude;
//     double d_xSwapAmplitudeaShift;
//     double d_xMovePhaseShift;
    double d_xMoveAmplitude;
//     double d_xMoveAmplitudeShift;
//     double d_ySwapPhaseShift;
    double d_ySwapAmplitude;
//     double d_ySwapAmplitudeShift;
//     double d_yMovePhaseShift;
    double d_yMoveAmplitude;
    double d_yMoveAmplitudeShift;
//     double d_zSwapPhaseShift;
    double d_zSwapAmplitude;
    double d_zSwapAmplitudeShift;
//     double d_zMovePhaseShift;
    double d_zMoveAmplitude;
    double d_zMoveAmplitudeShift;
//     double d_aMovePhaseShift;
    double d_aMoveAmplitude;
    double d_aMoveAmplitudeShift;

    double d_pelvisOffset;
    double d_pelvisSwing;
    double d_armSwingGain;

    double d_time;

    int    d_phase;
    double d_bodySwingY;
    double d_bodySwingZ;

    bool d_canStopNow;

    std::array<int,14> d_outValue;

    // initial pose
    Setting<double>* X_OFFSET;
    Setting<double>* Y_OFFSET;
    Setting<double>* Z_OFFSET;
    Setting<double>* YAW_OFFSET;
    Setting<double>* PITCH_OFFSET;
    Setting<double>* ROLL_OFFSET;

    // control
    Setting<double>* PERIOD_TIME;
    Setting<double>* DSP_RATIO;
    Setting<double>* STEP_FB_RATIO;
    Setting<double>* Z_MOVE_AMPLITUDE;

    // Balance control
    Setting<bool>* BALANCE_ENABLE;
    Setting<double>* BALANCE_KNEE_GAIN;
    Setting<double>* BALANCE_ANKLE_PITCH_GAIN;
    Setting<double>* BALANCE_HIP_ROLL_GAIN;
    Setting<double>* BALANCE_ANKLE_ROLL_GAIN;
    Setting<double>* Y_SWAP_AMPLITUDE;
    Setting<double>* Z_SWAP_AMPLITUDE;
    Setting<double>* ARM_SWING_GAIN;
    Setting<double>* PELVIS_OFFSET;

    Setting<int>* d_legGainP;
    Setting<int>* d_legGainI;
    Setting<int>* d_legGainD;
    Setting<int>* d_armGainP;
    Setting<int>* d_armGainI;
    Setting<int>* d_armGainD;
    Setting<int>* d_headGainP;
    Setting<int>* d_headGainI;
    Setting<int>* d_headGainD;
  };
}
