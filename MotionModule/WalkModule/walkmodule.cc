#include "walkmodule.hh"

#include <cmath>
#include <iostream>

#include "../../AgentState/agentstate.hh"
#include "../../BodyControl/bodycontrol.hh"
#include "../../CM730Snapshot/cm730snapshot.hh"
#include "../../minIni/minIni.h"
#include "../../MX28/mx28.hh"
#include "../../StateObject/HardwareState/hardwarestate.hh"

// TODO if we keep this walk, reimplement using Eigen and get rid of these classes:
#include "Vector.h"
#include "Matrix.h"

using namespace bold;
using namespace std;

WalkModule::WalkModule(minIni const& ini)
{
  string section = "Walk Module";

  X_OFFSET = ini.getd(section, "x_offset", -10);
  Y_OFFSET = ini.getd(section, "y_offset", 5);
  Z_OFFSET = ini.getd(section, "z_offset", 20);
  R_OFFSET = ini.getd(section, "roll_offset", 0);
  P_OFFSET = ini.getd(section, "pitch_offset", 0);
  A_OFFSET = ini.getd(section, "yaw_offset", 0);
  HIP_PITCH_OFFSET  = ini.getd(section, "hip_pitch_offset", 13.0);
  PERIOD_TIME       = ini.getd(section, "period_time", 600);
  DSP_RATIO         = ini.getd(section, "dsp_ratio", 0.1);
  STEP_FB_RATIO     = ini.getd(section, "step_forward_back_ratio", 0.28);
  Z_MOVE_AMPLITUDE  = ini.getd(section, "foot_height", 40);
  Y_SWAP_AMPLITUDE  = ini.getd(section, "swing_right_left", 20.0);
  Z_SWAP_AMPLITUDE  = ini.getd(section, "swing_top_down", 5);
  PELVIS_OFFSET     = ini.getd(section, "pelvis_offset", 3.0);
  ARM_SWING_GAIN    = ini.getd(section, "arm_swing_gain", 1.5);
  BALANCE_KNEE_GAIN        = ini.getd(section, "balance_knee_gain", 0.3);
  BALANCE_ANKLE_PITCH_GAIN = ini.getd(section, "balance_ankle_pitch_gain", 0.9);
  BALANCE_HIP_ROLL_GAIN    = ini.getd(section, "balance_hip_roll_gain", 0.5);
  BALANCE_ANKLE_ROLL_GAIN  = ini.getd(section, "balance_ankle_roll_gain", 1.0);

  P_GAIN = JointControl::P_GAIN_DEFAULT;
  I_GAIN = JointControl::I_GAIN_DEFAULT;
  D_GAIN = JointControl::D_GAIN_DEFAULT;

  X_MOVE_AMPLITUDE = 0;
  Y_MOVE_AMPLITUDE = 0;
  A_MOVE_AMPLITUDE = 0;
  A_MOVE_AIM_ON = false;
  BALANCE_ENABLE = true;

//   d_jointData.setAngle(JointControl::ID_R_SHOULDER_PITCH, -48.345);
//   d_jointData.setAngle(JointControl::ID_L_SHOULDER_PITCH, 41.313);
//   d_jointData.setAngle(JointControl::ID_R_SHOULDER_ROLL, -17.873);
//   d_jointData.setAngle(JointControl::ID_L_SHOULDER_ROLL, 17.580);
//   d_jointData.setAngle(JointControl::ID_R_ELBOW, 29.300);
//   d_jointData.setAngle(JointControl::ID_L_ELBOW, -29.593);
//
//   d_jointData.setAngle(JointControl::ID_HEAD_TILT, EYE_TILT_OFFSET_ANGLE);
//
//   d_jointData.setPGain(JointControl::ID_R_SHOULDER_PITCH, 8);
//   d_jointData.setPGain(JointControl::ID_L_SHOULDER_PITCH, 8);
//   d_jointData.setPGain(JointControl::ID_R_SHOULDER_ROLL, 8);
//   d_jointData.setPGain(JointControl::ID_L_SHOULDER_ROLL, 8);
//   d_jointData.setPGain(JointControl::ID_R_ELBOW, 8);
//   d_jointData.setPGain(JointControl::ID_L_ELBOW, 8);
}

WalkModule::~WalkModule()
{}

constexpr double WalkModule::EYE_TILT_OFFSET_ANGLE;
constexpr double WalkModule::THIGH_LENGTH;
constexpr double WalkModule::CALF_LENGTH;
constexpr double WalkModule::ANKLE_LENGTH;
constexpr double WalkModule::LEG_LENGTH;

double WalkModule::wsin(double time, double period, double period_shift, double mag, double mag_shift)
{
  return mag * sin(2 * M_PI / period * time - period_shift) + mag_shift;
}

bool WalkModule::computeIK(double *out, double x, double y, double z, double a, double b, double c)
{
  Matrix3D Tad, Tda, Tcd, Tdc, Tac;
  Vector3D vec;
  double Rac, Acos, Atan, k, l, m, n, s, _c;

  Tad.SetTransform(Point3D(x, y, z - LEG_LENGTH), Vector3D(a * 180.0 / M_PI, b * 180.0 / M_PI, c * 180.0 / M_PI));

  vec.X = x + Tad.m[2] * ANKLE_LENGTH;
  vec.Y = y + Tad.m[6] * ANKLE_LENGTH;
  vec.Z = (z - LEG_LENGTH) + Tad.m[10] * ANKLE_LENGTH;

  // Get Knee
  Rac = vec.Length();
  Acos = acos((Rac * Rac - THIGH_LENGTH * THIGH_LENGTH - CALF_LENGTH * CALF_LENGTH) / (2 * THIGH_LENGTH * CALF_LENGTH));

  if (std::isnan(Acos))
    return false;
  *(out + 3) = Acos;

  // Get Ankle Roll
  Tda = Tad;
  if (!Tda.Inverse())
    return false;
  k = sqrt(Tda.m[7] * Tda.m[7] + Tda.m[11] * Tda.m[11]);
  l = sqrt(Tda.m[7] * Tda.m[7] + (Tda.m[11] - ANKLE_LENGTH) * (Tda.m[11] - ANKLE_LENGTH));
  m = (k * k - l * l - ANKLE_LENGTH * ANKLE_LENGTH) / (2 * l * ANKLE_LENGTH);
  if (m > 1.0)
    m = 1.0;
  else if (m < -1.0)
    m = -1.0;
  Acos = acos(m);
  if (std::isnan(Acos))
    return false;
  if (Tda.m[7] < 0.0)
    *(out + 5) = -Acos;
  else
    *(out + 5) = Acos;

  // Get Hip Yaw
  Tcd.SetTransform(Point3D(0, 0, -ANKLE_LENGTH), Vector3D(*(out + 5) * 180.0 / M_PI, 0, 0));
  Tdc = Tcd;
  if (!Tdc.Inverse())
    return false;
  Tac = Tad * Tdc;
  Atan = atan2(-Tac.m[1] , Tac.m[5]);
  if (std::isinf(Atan))
    return false;
  *(out) = Atan;

  // Get Hip Roll
  Atan = atan2(Tac.m[9], -Tac.m[1] * sin(*(out)) + Tac.m[5] * cos(*(out)));
  if (std::isinf(Atan))
    return false;
  *(out + 1) = Atan;

  // Get Hip Pitch and Ankle Pitch
  Atan = atan2(Tac.m[2] * cos(*(out)) + Tac.m[6] * sin(*(out)), Tac.m[0] * cos(*(out)) + Tac.m[4] * sin(*(out)));
  if (std::isinf(Atan))
    return false;
  double theta = Atan;
  k = sin(*(out + 3)) * CALF_LENGTH;
  l = -THIGH_LENGTH - cos(*(out + 3)) * CALF_LENGTH;
  m = cos(*(out)) * vec.X + sin(*(out)) * vec.Y;
  n = cos(*(out + 1)) * vec.Z + sin(*(out)) * sin(*(out + 1)) * vec.X - cos(*(out)) * sin(*(out + 1)) * vec.Y;
  s = (k * n + l * m) / (k * k + l * l);
  _c = (n - k * s) / l;
  Atan = atan2(s, _c);
  if (std::isinf(Atan))
    return false;
  *(out + 2) = Atan;
  *(out + 4) = theta - *(out + 3) - *(out + 2);

  return true;
}

void WalkModule::updateTimeParams()
{
  m_PeriodTime = PERIOD_TIME;
  m_DSP_Ratio = DSP_RATIO;
  m_SSP_Ratio = 1 - DSP_RATIO;

  m_X_Swap_PeriodTime = m_PeriodTime / 2;
  m_X_Move_PeriodTime = m_PeriodTime * m_SSP_Ratio;
  m_Y_Swap_PeriodTime = m_PeriodTime;
  m_Y_Move_PeriodTime = m_PeriodTime * m_SSP_Ratio;
  m_Z_Swap_PeriodTime = m_PeriodTime / 2;
  m_Z_Move_PeriodTime = m_PeriodTime * m_SSP_Ratio / 2;
  m_A_Move_PeriodTime = m_PeriodTime * m_SSP_Ratio;

  m_SSP_Time = m_PeriodTime * m_SSP_Ratio;
  m_SSP_Time_Start_L = (1 - m_SSP_Ratio) * m_PeriodTime / 4;
  m_SSP_Time_End_L = (1 + m_SSP_Ratio) * m_PeriodTime / 4;
  m_SSP_Time_Start_R = (3 - m_SSP_Ratio) * m_PeriodTime / 4;
  m_SSP_Time_End_R = (3 + m_SSP_Ratio) * m_PeriodTime / 4;

  m_Phase_Time1 = (m_SSP_Time_End_L + m_SSP_Time_Start_L) / 2;
  m_Phase_Time2 = (m_SSP_Time_Start_R + m_SSP_Time_End_L) / 2;
  m_Phase_Time3 = (m_SSP_Time_End_R + m_SSP_Time_Start_R) / 2;

  m_Pelvis_Offset = PELVIS_OFFSET*MX28::RATIO_DEGS2VALUE;
  m_Pelvis_Swing = m_Pelvis_Offset * 0.35;
  m_Arm_Swing_Gain = ARM_SWING_GAIN;
}

void WalkModule::updateMovementParams()
{
  // Forward/Back
  m_X_Move_Amplitude = X_MOVE_AMPLITUDE;
  m_X_Swap_Amplitude = X_MOVE_AMPLITUDE * STEP_FB_RATIO;

  // Right/Left
  m_Y_Move_Amplitude = Y_MOVE_AMPLITUDE / 2;
  if (m_Y_Move_Amplitude > 0)
    m_Y_Move_Amplitude_Shift = m_Y_Move_Amplitude;
  else
    m_Y_Move_Amplitude_Shift = -m_Y_Move_Amplitude;
  m_Y_Swap_Amplitude = Y_SWAP_AMPLITUDE + m_Y_Move_Amplitude_Shift * 0.04;

  m_Z_Move_Amplitude = Z_MOVE_AMPLITUDE / 2;
  m_Z_Move_Amplitude_Shift = m_Z_Move_Amplitude / 2;
  m_Z_Swap_Amplitude = Z_SWAP_AMPLITUDE;
  m_Z_Swap_Amplitude_Shift = m_Z_Swap_Amplitude;

  // Direction
  if (!A_MOVE_AIM_ON)
  {
    m_A_Move_Amplitude = A_MOVE_AMPLITUDE * M_PI / 180.0 / 2;
    if (m_A_Move_Amplitude > 0)
      m_A_Move_Amplitude_Shift = m_A_Move_Amplitude;
    else
      m_A_Move_Amplitude_Shift = -m_A_Move_Amplitude;
  }
  else
  {
    m_A_Move_Amplitude = -A_MOVE_AMPLITUDE * M_PI / 180.0 / 2;
    if (m_A_Move_Amplitude > 0)
      m_A_Move_Amplitude_Shift = -m_A_Move_Amplitude;
    else
      m_A_Move_Amplitude_Shift = m_A_Move_Amplitude;
  }
}

void WalkModule::updateBalanceParams()
{
  m_X_Offset = X_OFFSET;
  m_Y_Offset = Y_OFFSET;
  m_Z_Offset = Z_OFFSET;
  m_R_Offset = R_OFFSET * M_PI / 180.0;
  m_P_Offset = P_OFFSET * M_PI / 180.0;
  m_A_Offset = A_OFFSET * M_PI / 180.0;
  m_Hip_Pitch_Offset = HIP_PITCH_OFFSET*MX28::RATIO_DEGS2VALUE;
}

void WalkModule::initialize()
{
  X_MOVE_AMPLITUDE   = 0;
  Y_MOVE_AMPLITUDE   = 0;
  A_MOVE_AMPLITUDE   = 0;

  m_Body_Swing_Y = 0;
  m_Body_Swing_Z = 0;

  m_X_Swap_Phase_Shift = M_PI;
  m_X_Swap_Amplitude_Shift = 0;
  m_X_Move_Phase_Shift = M_PI / 2;
  m_X_Move_Amplitude_Shift = 0;
  m_Y_Swap_Phase_Shift = 0;
  m_Y_Swap_Amplitude_Shift = 0;
  m_Y_Move_Phase_Shift = M_PI / 2;
  m_Z_Swap_Phase_Shift = M_PI * 3 / 2;
  m_Z_Move_Phase_Shift = M_PI / 2;
  m_A_Move_Phase_Shift = M_PI / 2;

  m_Ctrl_Running = false;
  m_Real_Running = false;
  m_Time = 0;
  updateTimeParams();
  updateMovementParams();

  step(JointSelection(true, true, true));
}

void WalkModule::start()
{
  m_Ctrl_Running = true;
  m_Real_Running = true;
}

void WalkModule::stop()
{
  m_Ctrl_Running = false;
}

bool WalkModule::isRunning()
{
  return m_Real_Running;
}

void WalkModule::step(JointSelection const& selectedJoints)
{
  double x_swap, y_swap, z_swap, a_swap, b_swap, c_swap;
  double x_move_r, y_move_r, z_move_r, a_move_r, b_move_r, c_move_r;
  double x_move_l, y_move_l, z_move_l, a_move_l, b_move_l, c_move_l;
  double pelvis_offset_r, pelvis_offset_l;
  double angle[14], ep[12];
  double offset;
  double TIME_UNIT = MotionModule::TIME_UNIT;
  //                     R_HIP_YAW, R_HIP_ROLL, R_HIP_PITCH, R_KNEE, R_ANKLE_PITCH, R_ANKLE_ROLL, L_HIP_YAW, L_HIP_ROLL, L_HIP_PITCH, L_KNEE, L_ANKLE_PITCH, L_ANKLE_ROLL, R_ARM_SWING, L_ARM_SWING
  int dir[14]          = {   -1,        -1,          1,         1,         -1,            1,          -1,        -1,         -1,         -1,         1,            1,           1,           -1      };
  double initAngle[14] = {   0.0,       0.0,        0.0,       0.0,        0.0,          0.0,         0.0,       0.0,        0.0,        0.0,       0.0,          0.0,       -48.345,       41.313    };

  // Update walk parameters
  if (m_Time == 0)
  {
    updateTimeParams();
    m_Phase = PHASE0;
    if (!m_Ctrl_Running)
    {
      if (m_X_Move_Amplitude == 0 && m_Y_Move_Amplitude == 0 && m_A_Move_Amplitude == 0)
      {
        m_Real_Running = false;
      }
      else
      {
        X_MOVE_AMPLITUDE = 0;
        Y_MOVE_AMPLITUDE = 0;
        A_MOVE_AMPLITUDE = 0;
      }
    }
  }
  else if (m_Time >= (m_Phase_Time1 - TIME_UNIT/2) && m_Time < (m_Phase_Time1 + TIME_UNIT/2))
  {
    updateMovementParams();
    m_Phase = PHASE1;
  }
  else if (m_Time >= (m_Phase_Time2 - TIME_UNIT/2) && m_Time < (m_Phase_Time2 + TIME_UNIT/2))
  {
    updateTimeParams();
    m_Time = m_Phase_Time2;
    m_Phase = PHASE2;
    if (!m_Ctrl_Running)
    {
      if (m_X_Move_Amplitude == 0 && m_Y_Move_Amplitude == 0 && m_A_Move_Amplitude == 0)
      {
        m_Real_Running = false;
      }
      else
      {
        X_MOVE_AMPLITUDE = 0;
        Y_MOVE_AMPLITUDE = 0;
        A_MOVE_AMPLITUDE = 0;
      }
    }
  }
  else if (m_Time >= (m_Phase_Time3 - TIME_UNIT/2) && m_Time < (m_Phase_Time3 + TIME_UNIT/2))
  {
    updateMovementParams();
    m_Phase = PHASE3;
  }
  updateBalanceParams();

  // Compute endpoints
  x_swap = wsin(m_Time, m_X_Swap_PeriodTime, m_X_Swap_Phase_Shift, m_X_Swap_Amplitude, m_X_Swap_Amplitude_Shift);
  y_swap = wsin(m_Time, m_Y_Swap_PeriodTime, m_Y_Swap_Phase_Shift, m_Y_Swap_Amplitude, m_Y_Swap_Amplitude_Shift);
  z_swap = wsin(m_Time, m_Z_Swap_PeriodTime, m_Z_Swap_Phase_Shift, m_Z_Swap_Amplitude, m_Z_Swap_Amplitude_Shift);
  a_swap = 0;
  b_swap = 0;
  c_swap = 0;

  if (m_Time <= m_SSP_Time_Start_L)
  {
    x_move_l = wsin(m_SSP_Time_Start_L, m_X_Move_PeriodTime, m_X_Move_Phase_Shift + 2 * M_PI / m_X_Move_PeriodTime * m_SSP_Time_Start_L, m_X_Move_Amplitude, m_X_Move_Amplitude_Shift);
    y_move_l = wsin(m_SSP_Time_Start_L, m_Y_Move_PeriodTime, m_Y_Move_Phase_Shift + 2 * M_PI / m_Y_Move_PeriodTime * m_SSP_Time_Start_L, m_Y_Move_Amplitude, m_Y_Move_Amplitude_Shift);
    z_move_l = wsin(m_SSP_Time_Start_L, m_Z_Move_PeriodTime, m_Z_Move_Phase_Shift + 2 * M_PI / m_Z_Move_PeriodTime * m_SSP_Time_Start_L, m_Z_Move_Amplitude, m_Z_Move_Amplitude_Shift);
    c_move_l = wsin(m_SSP_Time_Start_L, m_A_Move_PeriodTime, m_A_Move_Phase_Shift + 2 * M_PI / m_A_Move_PeriodTime * m_SSP_Time_Start_L, m_A_Move_Amplitude, m_A_Move_Amplitude_Shift);
    x_move_r = wsin(m_SSP_Time_Start_L, m_X_Move_PeriodTime, m_X_Move_Phase_Shift + 2 * M_PI / m_X_Move_PeriodTime * m_SSP_Time_Start_L, -m_X_Move_Amplitude, -m_X_Move_Amplitude_Shift);
    y_move_r = wsin(m_SSP_Time_Start_L, m_Y_Move_PeriodTime, m_Y_Move_Phase_Shift + 2 * M_PI / m_Y_Move_PeriodTime * m_SSP_Time_Start_L, -m_Y_Move_Amplitude, -m_Y_Move_Amplitude_Shift);
    z_move_r = wsin(m_SSP_Time_Start_R, m_Z_Move_PeriodTime, m_Z_Move_Phase_Shift + 2 * M_PI / m_Z_Move_PeriodTime * m_SSP_Time_Start_R, m_Z_Move_Amplitude, m_Z_Move_Amplitude_Shift);
    c_move_r = wsin(m_SSP_Time_Start_L, m_A_Move_PeriodTime, m_A_Move_Phase_Shift + 2 * M_PI / m_A_Move_PeriodTime * m_SSP_Time_Start_L, -m_A_Move_Amplitude, -m_A_Move_Amplitude_Shift);
    pelvis_offset_l = 0;
    pelvis_offset_r = 0;
  }
  else if (m_Time <= m_SSP_Time_End_L)
  {
    x_move_l = wsin(m_Time, m_X_Move_PeriodTime, m_X_Move_Phase_Shift + 2 * M_PI / m_X_Move_PeriodTime * m_SSP_Time_Start_L, m_X_Move_Amplitude, m_X_Move_Amplitude_Shift);
    y_move_l = wsin(m_Time, m_Y_Move_PeriodTime, m_Y_Move_Phase_Shift + 2 * M_PI / m_Y_Move_PeriodTime * m_SSP_Time_Start_L, m_Y_Move_Amplitude, m_Y_Move_Amplitude_Shift);
    z_move_l = wsin(m_Time, m_Z_Move_PeriodTime, m_Z_Move_Phase_Shift + 2 * M_PI / m_Z_Move_PeriodTime * m_SSP_Time_Start_L, m_Z_Move_Amplitude, m_Z_Move_Amplitude_Shift);
    c_move_l = wsin(m_Time, m_A_Move_PeriodTime, m_A_Move_Phase_Shift + 2 * M_PI / m_A_Move_PeriodTime * m_SSP_Time_Start_L, m_A_Move_Amplitude, m_A_Move_Amplitude_Shift);
    x_move_r = wsin(m_Time, m_X_Move_PeriodTime, m_X_Move_Phase_Shift + 2 * M_PI / m_X_Move_PeriodTime * m_SSP_Time_Start_L, -m_X_Move_Amplitude, -m_X_Move_Amplitude_Shift);
    y_move_r = wsin(m_Time, m_Y_Move_PeriodTime, m_Y_Move_Phase_Shift + 2 * M_PI / m_Y_Move_PeriodTime * m_SSP_Time_Start_L, -m_Y_Move_Amplitude, -m_Y_Move_Amplitude_Shift);
    z_move_r = wsin(m_SSP_Time_Start_R, m_Z_Move_PeriodTime, m_Z_Move_Phase_Shift + 2 * M_PI / m_Z_Move_PeriodTime * m_SSP_Time_Start_R, m_Z_Move_Amplitude, m_Z_Move_Amplitude_Shift);
    c_move_r = wsin(m_Time, m_A_Move_PeriodTime, m_A_Move_Phase_Shift + 2 * M_PI / m_A_Move_PeriodTime * m_SSP_Time_Start_L, -m_A_Move_Amplitude, -m_A_Move_Amplitude_Shift);
    pelvis_offset_l = wsin(m_Time, m_Z_Move_PeriodTime, m_Z_Move_Phase_Shift + 2 * M_PI / m_Z_Move_PeriodTime * m_SSP_Time_Start_L, m_Pelvis_Swing / 2, m_Pelvis_Swing / 2);
    pelvis_offset_r = wsin(m_Time, m_Z_Move_PeriodTime, m_Z_Move_Phase_Shift + 2 * M_PI / m_Z_Move_PeriodTime * m_SSP_Time_Start_L, -m_Pelvis_Offset / 2, -m_Pelvis_Offset / 2);
  }
  else if (m_Time <= m_SSP_Time_Start_R)
  {
    x_move_l = wsin(m_SSP_Time_End_L, m_X_Move_PeriodTime, m_X_Move_Phase_Shift + 2 * M_PI / m_X_Move_PeriodTime * m_SSP_Time_Start_L, m_X_Move_Amplitude, m_X_Move_Amplitude_Shift);
    y_move_l = wsin(m_SSP_Time_End_L, m_Y_Move_PeriodTime, m_Y_Move_Phase_Shift + 2 * M_PI / m_Y_Move_PeriodTime * m_SSP_Time_Start_L, m_Y_Move_Amplitude, m_Y_Move_Amplitude_Shift);
    z_move_l = wsin(m_SSP_Time_End_L, m_Z_Move_PeriodTime, m_Z_Move_Phase_Shift + 2 * M_PI / m_Z_Move_PeriodTime * m_SSP_Time_Start_L, m_Z_Move_Amplitude, m_Z_Move_Amplitude_Shift);
    c_move_l = wsin(m_SSP_Time_End_L, m_A_Move_PeriodTime, m_A_Move_Phase_Shift + 2 * M_PI / m_A_Move_PeriodTime * m_SSP_Time_Start_L, m_A_Move_Amplitude, m_A_Move_Amplitude_Shift);
    x_move_r = wsin(m_SSP_Time_End_L, m_X_Move_PeriodTime, m_X_Move_Phase_Shift + 2 * M_PI / m_X_Move_PeriodTime * m_SSP_Time_Start_L, -m_X_Move_Amplitude, -m_X_Move_Amplitude_Shift);
    y_move_r = wsin(m_SSP_Time_End_L, m_Y_Move_PeriodTime, m_Y_Move_Phase_Shift + 2 * M_PI / m_Y_Move_PeriodTime * m_SSP_Time_Start_L, -m_Y_Move_Amplitude, -m_Y_Move_Amplitude_Shift);
    z_move_r = wsin(m_SSP_Time_Start_R, m_Z_Move_PeriodTime, m_Z_Move_Phase_Shift + 2 * M_PI / m_Z_Move_PeriodTime * m_SSP_Time_Start_R, m_Z_Move_Amplitude, m_Z_Move_Amplitude_Shift);
    c_move_r = wsin(m_SSP_Time_End_L, m_A_Move_PeriodTime, m_A_Move_Phase_Shift + 2 * M_PI / m_A_Move_PeriodTime * m_SSP_Time_Start_L, -m_A_Move_Amplitude, -m_A_Move_Amplitude_Shift);
    pelvis_offset_l = 0;
    pelvis_offset_r = 0;
  }
  else if (m_Time <= m_SSP_Time_End_R)
  {
    x_move_l = wsin(m_Time, m_X_Move_PeriodTime, m_X_Move_Phase_Shift + 2 * M_PI / m_X_Move_PeriodTime * m_SSP_Time_Start_R + M_PI, m_X_Move_Amplitude, m_X_Move_Amplitude_Shift);
    y_move_l = wsin(m_Time, m_Y_Move_PeriodTime, m_Y_Move_Phase_Shift + 2 * M_PI / m_Y_Move_PeriodTime * m_SSP_Time_Start_R + M_PI, m_Y_Move_Amplitude, m_Y_Move_Amplitude_Shift);
    z_move_l = wsin(m_SSP_Time_End_L, m_Z_Move_PeriodTime, m_Z_Move_Phase_Shift + 2 * M_PI / m_Z_Move_PeriodTime * m_SSP_Time_Start_L, m_Z_Move_Amplitude, m_Z_Move_Amplitude_Shift);
    c_move_l = wsin(m_Time, m_A_Move_PeriodTime, m_A_Move_Phase_Shift + 2 * M_PI / m_A_Move_PeriodTime * m_SSP_Time_Start_R + M_PI, m_A_Move_Amplitude, m_A_Move_Amplitude_Shift);
    x_move_r = wsin(m_Time, m_X_Move_PeriodTime, m_X_Move_Phase_Shift + 2 * M_PI / m_X_Move_PeriodTime * m_SSP_Time_Start_R + M_PI, -m_X_Move_Amplitude, -m_X_Move_Amplitude_Shift);
    y_move_r = wsin(m_Time, m_Y_Move_PeriodTime, m_Y_Move_Phase_Shift + 2 * M_PI / m_Y_Move_PeriodTime * m_SSP_Time_Start_R + M_PI, -m_Y_Move_Amplitude, -m_Y_Move_Amplitude_Shift);
    z_move_r = wsin(m_Time, m_Z_Move_PeriodTime, m_Z_Move_Phase_Shift + 2 * M_PI / m_Z_Move_PeriodTime * m_SSP_Time_Start_R, m_Z_Move_Amplitude, m_Z_Move_Amplitude_Shift);
    c_move_r = wsin(m_Time, m_A_Move_PeriodTime, m_A_Move_Phase_Shift + 2 * M_PI / m_A_Move_PeriodTime * m_SSP_Time_Start_R + M_PI, -m_A_Move_Amplitude, -m_A_Move_Amplitude_Shift);
    pelvis_offset_l = wsin(m_Time, m_Z_Move_PeriodTime, m_Z_Move_Phase_Shift + 2 * M_PI / m_Z_Move_PeriodTime * m_SSP_Time_Start_R, m_Pelvis_Offset / 2, m_Pelvis_Offset / 2);
    pelvis_offset_r = wsin(m_Time, m_Z_Move_PeriodTime, m_Z_Move_Phase_Shift + 2 * M_PI / m_Z_Move_PeriodTime * m_SSP_Time_Start_R, -m_Pelvis_Swing / 2, -m_Pelvis_Swing / 2);
  }
  else
  {
    x_move_l = wsin(m_SSP_Time_End_R, m_X_Move_PeriodTime, m_X_Move_Phase_Shift + 2 * M_PI / m_X_Move_PeriodTime * m_SSP_Time_Start_R + M_PI, m_X_Move_Amplitude, m_X_Move_Amplitude_Shift);
    y_move_l = wsin(m_SSP_Time_End_R, m_Y_Move_PeriodTime, m_Y_Move_Phase_Shift + 2 * M_PI / m_Y_Move_PeriodTime * m_SSP_Time_Start_R + M_PI, m_Y_Move_Amplitude, m_Y_Move_Amplitude_Shift);
    z_move_l = wsin(m_SSP_Time_End_L, m_Z_Move_PeriodTime, m_Z_Move_Phase_Shift + 2 * M_PI / m_Z_Move_PeriodTime * m_SSP_Time_Start_L, m_Z_Move_Amplitude, m_Z_Move_Amplitude_Shift);
    c_move_l = wsin(m_SSP_Time_End_R, m_A_Move_PeriodTime, m_A_Move_Phase_Shift + 2 * M_PI / m_A_Move_PeriodTime * m_SSP_Time_Start_R + M_PI, m_A_Move_Amplitude, m_A_Move_Amplitude_Shift);
    x_move_r = wsin(m_SSP_Time_End_R, m_X_Move_PeriodTime, m_X_Move_Phase_Shift + 2 * M_PI / m_X_Move_PeriodTime * m_SSP_Time_Start_R + M_PI, -m_X_Move_Amplitude, -m_X_Move_Amplitude_Shift);
    y_move_r = wsin(m_SSP_Time_End_R, m_Y_Move_PeriodTime, m_Y_Move_Phase_Shift + 2 * M_PI / m_Y_Move_PeriodTime * m_SSP_Time_Start_R + M_PI, -m_Y_Move_Amplitude, -m_Y_Move_Amplitude_Shift);
    z_move_r = wsin(m_SSP_Time_End_R, m_Z_Move_PeriodTime, m_Z_Move_Phase_Shift + 2 * M_PI / m_Z_Move_PeriodTime * m_SSP_Time_Start_R, m_Z_Move_Amplitude, m_Z_Move_Amplitude_Shift);
    c_move_r = wsin(m_SSP_Time_End_R, m_A_Move_PeriodTime, m_A_Move_Phase_Shift + 2 * M_PI / m_A_Move_PeriodTime * m_SSP_Time_Start_R + M_PI, -m_A_Move_Amplitude, -m_A_Move_Amplitude_Shift);
    pelvis_offset_l = 0;
    pelvis_offset_r = 0;
  }

  a_move_l = 0;
  b_move_l = 0;
  a_move_r = 0;
  b_move_r = 0;

  ep[0] = x_swap + x_move_r + m_X_Offset;
  ep[1] = y_swap + y_move_r - m_Y_Offset / 2;
  ep[2] = z_swap + z_move_r + m_Z_Offset;
  ep[3] = a_swap + a_move_r - m_R_Offset / 2;
  ep[4] = b_swap + b_move_r + m_P_Offset;
  ep[5] = c_swap + c_move_r - m_A_Offset / 2;
  ep[6] = x_swap + x_move_l + m_X_Offset;
  ep[7] = y_swap + y_move_l + m_Y_Offset / 2;
  ep[8] = z_swap + z_move_l + m_Z_Offset;
  ep[9] = a_swap + a_move_l + m_R_Offset / 2;
  ep[10] = b_swap + b_move_l + m_P_Offset;
  ep[11] = c_swap + c_move_l + m_A_Offset / 2;

  // Compute body swing
  if (m_Time <= m_SSP_Time_End_L)
  {
    m_Body_Swing_Y = -ep[7];
    m_Body_Swing_Z = ep[8];
  }
  else
  {
    m_Body_Swing_Y = -ep[1];
    m_Body_Swing_Z = ep[2];
  }
  m_Body_Swing_Z -= LEG_LENGTH;

  // Compute arm swing
  if (m_X_Move_Amplitude == 0)
  {
    angle[12] = 0; // Right
    angle[13] = 0; // Left
  }
  else
  {
    angle[12] = wsin(m_Time, m_PeriodTime, M_PI * 1.5, -m_X_Move_Amplitude * m_Arm_Swing_Gain, 0);
    angle[13] = wsin(m_Time, m_PeriodTime, M_PI * 1.5, m_X_Move_Amplitude * m_Arm_Swing_Gain, 0);
  }

  if (m_Real_Running)
  {
    m_Time += TIME_UNIT;
    if (m_Time >= m_PeriodTime)
      m_Time = 0;
  }

  // Compute angles
  if (computeIK(&angle[0], ep[0], ep[1], ep[2], ep[3], ep[4], ep[5]) != 1 ||
      computeIK(&angle[6], ep[6], ep[7], ep[8], ep[9], ep[10], ep[11]) != 1)
  {
    // Do not use angle;
    return;
  }

  // Convert leg angles from radians to degrees (skip shoulders and head pan)
  for (int i = 0; i < 12; i++)
    angle[i] *= 180.0 / M_PI;

  // Compute motor value
  for (int i = 0; i < 14; i++)
  {
    offset = (double)dir[i] * angle[i] * MX28::RATIO_DEGS2VALUE;
    if (i == 1) // R_HIP_ROLL
      offset += (double)dir[i] * pelvis_offset_r;
    else if (i == 7) // L_HIP_ROLL
      offset += (double)dir[i] * pelvis_offset_l;
    else if (i == 2 || i == 8) // R_HIP_PITCH or L_HIP_PITCH
      offset -= (double)dir[i] * HIP_PITCH_OFFSET * MX28::RATIO_DEGS2VALUE;

    d_outValue[i] = MX28::degs2Value(initAngle[i]) + (int)offset;
  }

  // adjust balance offset
  // TODO convert this stabilisation to a generic and replaceable BodyControlModulator ?
  if (BALANCE_ENABLE)
  {
    auto gryoRaw = AgentState::get<HardwareState>()->getCM730State()->gyroRaw;

    // TODO review the gyro axes labels
    double rlGyroErr = gryoRaw.x();
    double fbGyroErr = gryoRaw.y();

    // TODO remove this *4 but updating constants
    d_outValue[1]  += (int)(dir[1] * rlGyroErr * BALANCE_HIP_ROLL_GAIN*4); // R_HIP_ROLL
    d_outValue[7]  += (int)(dir[7] * rlGyroErr * BALANCE_HIP_ROLL_GAIN*4); // L_HIP_ROLL

    d_outValue[3]  -= (int)(dir[3] * fbGyroErr * BALANCE_KNEE_GAIN*4); // R_KNEE
    d_outValue[9]  -= (int)(dir[9] * fbGyroErr * BALANCE_KNEE_GAIN*4); // L_KNEE

    d_outValue[4]  -= (int)(dir[4]  * fbGyroErr * BALANCE_ANKLE_PITCH_GAIN*4); // R_ANKLE_PITCH
    d_outValue[10] -= (int)(dir[10] * fbGyroErr * BALANCE_ANKLE_PITCH_GAIN*4); // L_ANKLE_PITCH

    d_outValue[5]  -= (int)(dir[5]  * rlGyroErr * BALANCE_ANKLE_ROLL_GAIN*4); // R_ANKLE_ROLL
    d_outValue[11] -= (int)(dir[11] * rlGyroErr * BALANCE_ANKLE_ROLL_GAIN*4); // L_ANKLE_ROLL
  }

//   d_jointData.setValue(JointControl::ID_R_HIP_YAW,        d_outValue[0]);
//   d_jointData.setValue(JointControl::ID_R_HIP_ROLL,       d_outValue[1]);
//   d_jointData.setValue(JointControl::ID_R_HIP_PITCH,      d_outValue[2]);
//   d_jointData.setValue(JointControl::ID_R_KNEE,           d_outValue[3]);
//   d_jointData.setValue(JointControl::ID_R_ANKLE_PITCH,    d_outValue[4]);
//   d_jointData.setValue(JointControl::ID_R_ANKLE_ROLL,     d_outValue[5]);
//   d_jointData.setValue(JointControl::ID_L_HIP_YAW,        d_outValue[6]);
//   d_jointData.setValue(JointControl::ID_L_HIP_ROLL,       d_outValue[7]);
//   d_jointData.setValue(JointControl::ID_L_HIP_PITCH,      d_outValue[8]);
//   d_jointData.setValue(JointControl::ID_L_KNEE,           d_outValue[9]);
//   d_jointData.setValue(JointControl::ID_L_ANKLE_PITCH,    d_outValue[10]);
//   d_jointData.setValue(JointControl::ID_L_ANKLE_ROLL,     d_outValue[11]);
//   d_jointData.setValue(JointControl::ID_R_SHOULDER_PITCH, d_outValue[12]);
//   d_jointData.setValue(JointControl::ID_L_SHOULDER_PITCH, d_outValue[13]);
//   d_jointData.setAngle(JointControl::ID_HEAD_PAN,         A_MOVE_AMPLITUDE);
//
//   for (int id = JointControl::ID_R_HIP_YAW; id <= JointControl::ID_L_ANKLE_ROLL; id++)
//   {
//     d_jointData.setPGain(id, P_GAIN);
//     d_jointData.setIGain(id, I_GAIN);
//     d_jointData.setDGain(id, D_GAIN);
//   }
}

void WalkModule::applyHead(shared_ptr<HeadSection> head)
{
  // Ensure we have our standard PID values
  head->visitJoints([this](shared_ptr<JointControl> joint){ joint->setPidGains(P_GAIN, I_GAIN, D_GAIN); });

  head->pan()->setAngle(A_MOVE_AMPLITUDE);
}

void WalkModule::applyArms(shared_ptr<ArmSection> arms)
{
  // Ensure we have our standard PID values
  arms->visitJoints([this](shared_ptr<JointControl> joint){ joint->setPidGains(P_GAIN, I_GAIN, D_GAIN); });

  arms->shoulderPitchRight()->setValue(d_outValue[12]);
  arms->shoulderPitchLeft()->setValue(d_outValue[13]);
}

void WalkModule::applyLegs(shared_ptr<LegSection> legs)
{
  // Ensure we have our standard PID values
  legs->visitJoints([this](shared_ptr<JointControl> joint){ joint->setPidGains(P_GAIN, I_GAIN, D_GAIN); });

  legs->hipYawRight()->setValue(d_outValue[0]);
  legs->hipRollRight()->setValue(d_outValue[1]);
  legs->hipPitchRight()->setValue(d_outValue[2]);
  legs->kneeRight()->setValue(d_outValue[3]);
  legs->anklePitchRight()->setValue(d_outValue[4]);
  legs->ankleRollRight()->setValue(d_outValue[5]);
  legs->hipYawLeft()->setValue(d_outValue[6]);
  legs->hipRollLeft()->setValue(d_outValue[7]);
  legs->hipPitchLeft()->setValue(d_outValue[8]);
  legs->kneeLeft()->setValue(d_outValue[9]);
  legs->anklePitchLeft()->setValue(d_outValue[10]);
  legs->ankleRollLeft()->setValue(d_outValue[11]);
}
