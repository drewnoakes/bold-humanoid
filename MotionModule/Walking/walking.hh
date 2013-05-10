#pragma once

#include <string.h>

#include "../motionmodule.hh"

class minIni;

namespace bold
{
  class Walking : public MotionModule
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

    double m_PeriodTime;
    double m_DSP_Ratio;
    double m_SSP_Ratio;
    double m_X_Swap_PeriodTime;
    double m_X_Move_PeriodTime;
    double m_Y_Swap_PeriodTime;
    double m_Y_Move_PeriodTime;
    double m_Z_Swap_PeriodTime;
    double m_Z_Move_PeriodTime;
    double m_A_Move_PeriodTime;
    double m_SSP_Time;
    double m_SSP_Time_Start_L;
    double m_SSP_Time_End_L;
    double m_SSP_Time_Start_R;
    double m_SSP_Time_End_R;
    double m_Phase_Time1;
    double m_Phase_Time2;
    double m_Phase_Time3;

    double m_X_Offset;
    double m_Y_Offset;
    double m_Z_Offset;
    double m_R_Offset;
    double m_P_Offset;
    double m_A_Offset;

    double m_X_Swap_Phase_Shift;
    double m_X_Swap_Amplitude;
    double m_X_Swap_Amplitude_Shift;
    double m_X_Move_Phase_Shift;
    double m_X_Move_Amplitude;
    double m_X_Move_Amplitude_Shift;
    double m_Y_Swap_Phase_Shift;
    double m_Y_Swap_Amplitude;
    double m_Y_Swap_Amplitude_Shift;
    double m_Y_Move_Phase_Shift;
    double m_Y_Move_Amplitude;
    double m_Y_Move_Amplitude_Shift;
    double m_Z_Swap_Phase_Shift;
    double m_Z_Swap_Amplitude;
    double m_Z_Swap_Amplitude_Shift;
    double m_Z_Move_Phase_Shift;
    double m_Z_Move_Amplitude;
    double m_Z_Move_Amplitude_Shift;
    double m_A_Move_Phase_Shift;
    double m_A_Move_Amplitude;
    double m_A_Move_Amplitude_Shift;

    double m_Pelvis_Offset;
    double m_Pelvis_Swing;
    double m_Hip_Pitch_Offset;
    double m_Arm_Swing_Gain;

    bool m_Ctrl_Running;
    bool m_Real_Running;
    double m_Time;

    int    m_Phase;
    double m_Body_Swing_Y;
    double m_Body_Swing_Z;

    int d_outValue[14];

    static double wsin(double time, double period, double period_shift, double mag, double mag_shift);
    static bool computeIK(double *out, double x, double y, double z, double a, double b, double c);

    void updateTimeParams();
    void updateMovementParams();
    void updateBalanceParams();

  public:
    Walking(minIni const& ini);

    virtual ~Walking();

    // Walking initial pose
    double X_OFFSET;
    double Y_OFFSET;
    double Z_OFFSET;
    double A_OFFSET;
    double P_OFFSET;
    double R_OFFSET;

    // Walking control
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

    int getCurrentPhase() const  { return m_Phase; }
    double getBodySwingY() const { return m_Body_Swing_Y; }
    double getBodySwingZ() const { return m_Body_Swing_Z; }

    void start();
    void stop();
    bool isRunning();
  };
}
