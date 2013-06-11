#include "walkmodule.hh"

#include <cmath>
#include <iostream>

#include "../../AgentState/agentstate.hh"
#include "../../BodyControl/bodycontrol.hh"
#include "../../CM730Snapshot/cm730snapshot.hh"
#include "../../MX28/mx28.hh"
#include "../../StateObject/HardwareState/hardwarestate.hh"
#include "../../ThreadId/threadid.hh"

// TODO if we keep this walk, reimplement using Eigen and get rid of these classes:
#include "Vector.h"
#include "Matrix.h"

using namespace bold;
using namespace std;

WalkModule::WalkModule(std::shared_ptr<MotionTaskScheduler> scheduler)
: MotionModule("walk", scheduler)
{
  string section = "Walk Module";

  X_OFFSET          = getParam("x_offset", -10);
  Y_OFFSET          = getParam("y_offset", 5);
  Z_OFFSET          = getParam("z_offset", 20);
  R_OFFSET          = getParam("roll_offset", 0);
  P_OFFSET          = getParam("pitch_offset", 0);
  A_OFFSET          = getParam("yaw_offset", 0);
  HIP_PITCH_OFFSET  = getParam("hip_pitch_offset", 13.0);
  PERIOD_TIME       = getParam("period_time", 600);
  DSP_RATIO         = getParam("dsp_ratio", 0.1);
  STEP_FB_RATIO     = getParam("step_forward_back_ratio", 0.28);
  Z_MOVE_AMPLITUDE  = getParam("foot_height", 40);
  Y_SWAP_AMPLITUDE  = getParam("swing_right_left", 20.0);
  Z_SWAP_AMPLITUDE  = getParam("swing_top_down", 5);
  PELVIS_OFFSET     = getParam("pelvis_offset", 3.0);
  ARM_SWING_GAIN    = getParam("arm_swing_gain", 1.5);
  
  BALANCE_KNEE_GAIN        = getParam("balance_knee_gain", 1.2);
  BALANCE_ANKLE_PITCH_GAIN = getParam("balance_ankle_pitch_gain", 3.6);
  BALANCE_HIP_ROLL_GAIN    = getParam("balance_hip_roll_gain", 2.0);
  BALANCE_ANKLE_ROLL_GAIN  = getParam("balance_ankle_roll_gain", 4.0);

  P_GAIN = JointControl::P_GAIN_DEFAULT;
  I_GAIN = JointControl::I_GAIN_DEFAULT;
  D_GAIN = JointControl::D_GAIN_DEFAULT;

  X_MOVE_AMPLITUDE = 0;
  Y_MOVE_AMPLITUDE = 0;
  A_MOVE_AMPLITUDE = 0;
  A_MOVE_AIM_ON = false;
  BALANCE_ENABLE = true;
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
  d_periodTime = PERIOD_TIME;
  d_dspRatio = DSP_RATIO;
  d_sspRatio = 1 - DSP_RATIO;

  d_xSwapPeriodTime = d_periodTime / 2;
  d_xMovePeriodTime = d_periodTime * d_sspRatio;
  d_ySwapPeriodTime = d_periodTime;
  d_yMovePeriodTime = d_periodTime * d_sspRatio;
  d_zSwapPeriodTime = d_periodTime / 2;
  d_zMovePeriodTime = d_periodTime * d_sspRatio / 2;
  d_aMovePeriodTime = d_periodTime * d_sspRatio;

  d_sspTime = d_periodTime * d_sspRatio;
  d_sspTimeStartL = (1 - d_sspRatio) * d_periodTime / 4;
  d_sspTimeEndL = (1 + d_sspRatio) * d_periodTime / 4;
  d_sspTimeStartR = (3 - d_sspRatio) * d_periodTime / 4;
  d_sspTimeEndR = (3 + d_sspRatio) * d_periodTime / 4;

  d_phaseTime1 = (d_sspTimeEndL + d_sspTimeStartL) / 2;
  d_phaseTime2 = (d_sspTimeStartR + d_sspTimeEndL) / 2;
  d_phaseTime3 = (d_sspTimeEndR + d_sspTimeStartR) / 2;

  d_pelvisOffset = PELVIS_OFFSET*MX28::RATIO_DEGS2VALUE;
  d_pelvisSwing = d_pelvisOffset * 0.35;
  d_armSwingGain = ARM_SWING_GAIN;
}

void WalkModule::updateMovementParams()
{
  // Forward/Back
  d_xMoveAmplitude = X_MOVE_AMPLITUDE;
  d_xSwapAmplitude = X_MOVE_AMPLITUDE * STEP_FB_RATIO;

  // Right/Left
  d_yMoveAmplitude = Y_MOVE_AMPLITUDE / 2;
  if (d_yMoveAmplitude > 0)
    d_yMoveAmplitudeShift = d_yMoveAmplitude;
  else
    d_yMoveAmplitudeShift = -d_yMoveAmplitude;
  d_ySwapAmplitude = Y_SWAP_AMPLITUDE + d_yMoveAmplitudeShift * 0.04;

  d_zMoveAmplitude = Z_MOVE_AMPLITUDE / 2;
  d_zMoveAmplitudeShift = d_zMoveAmplitude / 2;
  d_zSwapAmplitude = Z_SWAP_AMPLITUDE;
  d_zSwapAmplitudeShift = d_zSwapAmplitude;

  // Direction
  if (!A_MOVE_AIM_ON)
  {
    d_aMoveAmplitude = A_MOVE_AMPLITUDE * M_PI / 180.0 / 2;
    if (d_aMoveAmplitude > 0)
      d_aMoveAmplitudeShift = d_aMoveAmplitude;
    else
      d_aMoveAmplitudeShift = -d_aMoveAmplitude;
  }
  else
  {
    d_aMoveAmplitude = -A_MOVE_AMPLITUDE * M_PI / 180.0 / 2;
    if (d_aMoveAmplitude > 0)
      d_aMoveAmplitudeShift = -d_aMoveAmplitude;
    else
      d_aMoveAmplitudeShift = d_aMoveAmplitude;
  }
}

void WalkModule::updateBalanceParams()
{
  d_xOffset = X_OFFSET;
  d_yOffset = Y_OFFSET;
  d_zOffset = Z_OFFSET;
  d_rOffset = R_OFFSET * M_PI / 180.0;
  d_pOffset = P_OFFSET * M_PI / 180.0;
  d_aOffset = A_OFFSET * M_PI / 180.0;
  d_hipPitchOffset = HIP_PITCH_OFFSET*MX28::RATIO_DEGS2VALUE;
}

void WalkModule::initialize()
{
  X_MOVE_AMPLITUDE = 0;
  Y_MOVE_AMPLITUDE = 0;
  A_MOVE_AMPLITUDE = 0;

  d_bodySwingY = 0;
  d_bodySwingZ = 0;

  d_xSwapPhaseShift = M_PI;
  d_xSwapAmplitudeaShift = 0;
  d_xMovePhaseShift = M_PI / 2;
  d_xMoveAmplitudeShift = 0;
  d_ySwapPhaseShift = 0;
  d_ySwapAmplitudeShift = 0;
  d_yMovePhaseShift = M_PI / 2;
  d_zSwapPhaseShift = M_PI * 3 / 2;
  d_zMovePhaseShift = M_PI / 2;
  d_aMovePhaseShift = M_PI / 2;

  d_isStopRequested = false;
  d_isRunning = false;
  d_time = 0;
  updateTimeParams();
  updateMovementParams();
}

void WalkModule::start()
{
  d_isStopRequested = false;
  d_isRunning = true;
}

void WalkModule::stop()
{
  d_isStopRequested = true;
}

bool WalkModule::isRunning()
{
  return d_isRunning;
}

void WalkModule::step(shared_ptr<JointSelection> selectedJoints)
{
  assert(ThreadId::isMotionLoopThread());

  // Update walk parameters
  if (d_time == 0)
  {
    updateTimeParams();
    d_phase = PHASE0;
    if (d_isStopRequested)
    {
      if (d_xMoveAmplitude == 0 && d_yMoveAmplitude == 0 && d_aMoveAmplitude == 0)
      {
        d_isRunning = false;
        setCompletedFlag();
      }
      else
      {
        X_MOVE_AMPLITUDE = 0;
        Y_MOVE_AMPLITUDE = 0;
        A_MOVE_AMPLITUDE = 0;
      }
    }
  }
  else if (d_time >= (d_phaseTime1 - TIME_UNIT/2) && d_time < (d_phaseTime1 + TIME_UNIT/2))
  {
    updateMovementParams();
    d_phase = PHASE1;
  }
  else if (d_time >= (d_phaseTime2 - TIME_UNIT/2) && d_time < (d_phaseTime2 + TIME_UNIT/2))
  {
    updateTimeParams();
    d_time = d_phaseTime2;
    d_phase = PHASE2;
    if (d_isStopRequested)
    {
      if (d_xMoveAmplitude == 0 && d_yMoveAmplitude == 0 && d_aMoveAmplitude == 0)
      {
        d_isRunning = false;
        setCompletedFlag();
      }
      else
      {
        X_MOVE_AMPLITUDE = 0;
        Y_MOVE_AMPLITUDE = 0;
        A_MOVE_AMPLITUDE = 0;
      }
    }
  }
  else if (d_time >= (d_phaseTime3 - TIME_UNIT/2) && d_time < (d_phaseTime3 + TIME_UNIT/2))
  {
    updateMovementParams();
    d_phase = PHASE3;
  }
  updateBalanceParams();

  // Compute endpoints
  double x_swap = wsin(d_time, d_xSwapPeriodTime, d_xSwapPhaseShift, d_xSwapAmplitude, d_xSwapAmplitudeaShift);
  double y_swap = wsin(d_time, d_ySwapPeriodTime, d_ySwapPhaseShift, d_ySwapAmplitude, d_ySwapAmplitudeShift);
  double z_swap = wsin(d_time, d_zSwapPeriodTime, d_zSwapPhaseShift, d_zSwapAmplitude, d_zSwapAmplitudeShift);
  double a_swap = 0;
  double b_swap = 0;
  double c_swap = 0;

  double x_move_r, y_move_r, z_move_r, a_move_r, b_move_r, c_move_r;
  double x_move_l, y_move_l, z_move_l, a_move_l, b_move_l, c_move_l;
  double pelvis_offset_r, pelvis_offset_l;

  if (d_time <= d_sspTimeStartL)
  {
    x_move_l = wsin(d_sspTimeStartL, d_xMovePeriodTime, d_xMovePhaseShift + 2 * M_PI / d_xMovePeriodTime * d_sspTimeStartL, d_xMoveAmplitude, d_xMoveAmplitudeShift);
    y_move_l = wsin(d_sspTimeStartL, d_yMovePeriodTime, d_yMovePhaseShift + 2 * M_PI / d_yMovePeriodTime * d_sspTimeStartL, d_yMoveAmplitude, d_yMoveAmplitudeShift);
    z_move_l = wsin(d_sspTimeStartL, d_zMovePeriodTime, d_zMovePhaseShift + 2 * M_PI / d_zMovePeriodTime * d_sspTimeStartL, d_zMoveAmplitude, d_zMoveAmplitudeShift);
    c_move_l = wsin(d_sspTimeStartL, d_aMovePeriodTime, d_aMovePhaseShift + 2 * M_PI / d_aMovePeriodTime * d_sspTimeStartL, d_aMoveAmplitude, d_aMoveAmplitudeShift);
    x_move_r = wsin(d_sspTimeStartL, d_xMovePeriodTime, d_xMovePhaseShift + 2 * M_PI / d_xMovePeriodTime * d_sspTimeStartL, -d_xMoveAmplitude, -d_xMoveAmplitudeShift);
    y_move_r = wsin(d_sspTimeStartL, d_yMovePeriodTime, d_yMovePhaseShift + 2 * M_PI / d_yMovePeriodTime * d_sspTimeStartL, -d_yMoveAmplitude, -d_yMoveAmplitudeShift);
    z_move_r = wsin(d_sspTimeStartR, d_zMovePeriodTime, d_zMovePhaseShift + 2 * M_PI / d_zMovePeriodTime * d_sspTimeStartR, d_zMoveAmplitude, d_zMoveAmplitudeShift);
    c_move_r = wsin(d_sspTimeStartL, d_aMovePeriodTime, d_aMovePhaseShift + 2 * M_PI / d_aMovePeriodTime * d_sspTimeStartL, -d_aMoveAmplitude, -d_aMoveAmplitudeShift);
    pelvis_offset_l = 0;
    pelvis_offset_r = 0;
  }
  else if (d_time <= d_sspTimeEndL)
  {
    x_move_l = wsin(d_time, d_xMovePeriodTime, d_xMovePhaseShift + 2 * M_PI / d_xMovePeriodTime * d_sspTimeStartL, d_xMoveAmplitude, d_xMoveAmplitudeShift);
    y_move_l = wsin(d_time, d_yMovePeriodTime, d_yMovePhaseShift + 2 * M_PI / d_yMovePeriodTime * d_sspTimeStartL, d_yMoveAmplitude, d_yMoveAmplitudeShift);
    z_move_l = wsin(d_time, d_zMovePeriodTime, d_zMovePhaseShift + 2 * M_PI / d_zMovePeriodTime * d_sspTimeStartL, d_zMoveAmplitude, d_zMoveAmplitudeShift);
    c_move_l = wsin(d_time, d_aMovePeriodTime, d_aMovePhaseShift + 2 * M_PI / d_aMovePeriodTime * d_sspTimeStartL, d_aMoveAmplitude, d_aMoveAmplitudeShift);
    x_move_r = wsin(d_time, d_xMovePeriodTime, d_xMovePhaseShift + 2 * M_PI / d_xMovePeriodTime * d_sspTimeStartL, -d_xMoveAmplitude, -d_xMoveAmplitudeShift);
    y_move_r = wsin(d_time, d_yMovePeriodTime, d_yMovePhaseShift + 2 * M_PI / d_yMovePeriodTime * d_sspTimeStartL, -d_yMoveAmplitude, -d_yMoveAmplitudeShift);
    z_move_r = wsin(d_sspTimeStartR, d_zMovePeriodTime, d_zMovePhaseShift + 2 * M_PI / d_zMovePeriodTime * d_sspTimeStartR, d_zMoveAmplitude, d_zMoveAmplitudeShift);
    c_move_r = wsin(d_time, d_aMovePeriodTime, d_aMovePhaseShift + 2 * M_PI / d_aMovePeriodTime * d_sspTimeStartL, -d_aMoveAmplitude, -d_aMoveAmplitudeShift);
    pelvis_offset_l = wsin(d_time, d_zMovePeriodTime, d_zMovePhaseShift + 2 * M_PI / d_zMovePeriodTime * d_sspTimeStartL, d_pelvisSwing / 2, d_pelvisSwing / 2);
    pelvis_offset_r = wsin(d_time, d_zMovePeriodTime, d_zMovePhaseShift + 2 * M_PI / d_zMovePeriodTime * d_sspTimeStartL, -d_pelvisOffset / 2, -d_pelvisOffset / 2);
  }
  else if (d_time <= d_sspTimeStartR)
  {
    x_move_l = wsin(d_sspTimeEndL, d_xMovePeriodTime, d_xMovePhaseShift + 2 * M_PI / d_xMovePeriodTime * d_sspTimeStartL, d_xMoveAmplitude, d_xMoveAmplitudeShift);
    y_move_l = wsin(d_sspTimeEndL, d_yMovePeriodTime, d_yMovePhaseShift + 2 * M_PI / d_yMovePeriodTime * d_sspTimeStartL, d_yMoveAmplitude, d_yMoveAmplitudeShift);
    z_move_l = wsin(d_sspTimeEndL, d_zMovePeriodTime, d_zMovePhaseShift + 2 * M_PI / d_zMovePeriodTime * d_sspTimeStartL, d_zMoveAmplitude, d_zMoveAmplitudeShift);
    c_move_l = wsin(d_sspTimeEndL, d_aMovePeriodTime, d_aMovePhaseShift + 2 * M_PI / d_aMovePeriodTime * d_sspTimeStartL, d_aMoveAmplitude, d_aMoveAmplitudeShift);
    x_move_r = wsin(d_sspTimeEndL, d_xMovePeriodTime, d_xMovePhaseShift + 2 * M_PI / d_xMovePeriodTime * d_sspTimeStartL, -d_xMoveAmplitude, -d_xMoveAmplitudeShift);
    y_move_r = wsin(d_sspTimeEndL, d_yMovePeriodTime, d_yMovePhaseShift + 2 * M_PI / d_yMovePeriodTime * d_sspTimeStartL, -d_yMoveAmplitude, -d_yMoveAmplitudeShift);
    z_move_r = wsin(d_sspTimeStartR, d_zMovePeriodTime, d_zMovePhaseShift + 2 * M_PI / d_zMovePeriodTime * d_sspTimeStartR, d_zMoveAmplitude, d_zMoveAmplitudeShift);
    c_move_r = wsin(d_sspTimeEndL, d_aMovePeriodTime, d_aMovePhaseShift + 2 * M_PI / d_aMovePeriodTime * d_sspTimeStartL, -d_aMoveAmplitude, -d_aMoveAmplitudeShift);
    pelvis_offset_l = 0;
    pelvis_offset_r = 0;
  }
  else if (d_time <= d_sspTimeEndR)
  {
    x_move_l = wsin(d_time, d_xMovePeriodTime, d_xMovePhaseShift + 2 * M_PI / d_xMovePeriodTime * d_sspTimeStartR + M_PI, d_xMoveAmplitude, d_xMoveAmplitudeShift);
    y_move_l = wsin(d_time, d_yMovePeriodTime, d_yMovePhaseShift + 2 * M_PI / d_yMovePeriodTime * d_sspTimeStartR + M_PI, d_yMoveAmplitude, d_yMoveAmplitudeShift);
    z_move_l = wsin(d_sspTimeEndL, d_zMovePeriodTime, d_zMovePhaseShift + 2 * M_PI / d_zMovePeriodTime * d_sspTimeStartL, d_zMoveAmplitude, d_zMoveAmplitudeShift);
    c_move_l = wsin(d_time, d_aMovePeriodTime, d_aMovePhaseShift + 2 * M_PI / d_aMovePeriodTime * d_sspTimeStartR + M_PI, d_aMoveAmplitude, d_aMoveAmplitudeShift);
    x_move_r = wsin(d_time, d_xMovePeriodTime, d_xMovePhaseShift + 2 * M_PI / d_xMovePeriodTime * d_sspTimeStartR + M_PI, -d_xMoveAmplitude, -d_xMoveAmplitudeShift);
    y_move_r = wsin(d_time, d_yMovePeriodTime, d_yMovePhaseShift + 2 * M_PI / d_yMovePeriodTime * d_sspTimeStartR + M_PI, -d_yMoveAmplitude, -d_yMoveAmplitudeShift);
    z_move_r = wsin(d_time, d_zMovePeriodTime, d_zMovePhaseShift + 2 * M_PI / d_zMovePeriodTime * d_sspTimeStartR, d_zMoveAmplitude, d_zMoveAmplitudeShift);
    c_move_r = wsin(d_time, d_aMovePeriodTime, d_aMovePhaseShift + 2 * M_PI / d_aMovePeriodTime * d_sspTimeStartR + M_PI, -d_aMoveAmplitude, -d_aMoveAmplitudeShift);
    pelvis_offset_l = wsin(d_time, d_zMovePeriodTime, d_zMovePhaseShift + 2 * M_PI / d_zMovePeriodTime * d_sspTimeStartR, d_pelvisOffset / 2, d_pelvisOffset / 2);
    pelvis_offset_r = wsin(d_time, d_zMovePeriodTime, d_zMovePhaseShift + 2 * M_PI / d_zMovePeriodTime * d_sspTimeStartR, -d_pelvisSwing / 2, -d_pelvisSwing / 2);
  }
  else
  {
    x_move_l = wsin(d_sspTimeEndR, d_xMovePeriodTime, d_xMovePhaseShift + 2 * M_PI / d_xMovePeriodTime * d_sspTimeStartR + M_PI, d_xMoveAmplitude, d_xMoveAmplitudeShift);
    y_move_l = wsin(d_sspTimeEndR, d_yMovePeriodTime, d_yMovePhaseShift + 2 * M_PI / d_yMovePeriodTime * d_sspTimeStartR + M_PI, d_yMoveAmplitude, d_yMoveAmplitudeShift);
    z_move_l = wsin(d_sspTimeEndL, d_zMovePeriodTime, d_zMovePhaseShift + 2 * M_PI / d_zMovePeriodTime * d_sspTimeStartL, d_zMoveAmplitude, d_zMoveAmplitudeShift);
    c_move_l = wsin(d_sspTimeEndR, d_aMovePeriodTime, d_aMovePhaseShift + 2 * M_PI / d_aMovePeriodTime * d_sspTimeStartR + M_PI, d_aMoveAmplitude, d_aMoveAmplitudeShift);
    x_move_r = wsin(d_sspTimeEndR, d_xMovePeriodTime, d_xMovePhaseShift + 2 * M_PI / d_xMovePeriodTime * d_sspTimeStartR + M_PI, -d_xMoveAmplitude, -d_xMoveAmplitudeShift);
    y_move_r = wsin(d_sspTimeEndR, d_yMovePeriodTime, d_yMovePhaseShift + 2 * M_PI / d_yMovePeriodTime * d_sspTimeStartR + M_PI, -d_yMoveAmplitude, -d_yMoveAmplitudeShift);
    z_move_r = wsin(d_sspTimeEndR, d_zMovePeriodTime, d_zMovePhaseShift + 2 * M_PI / d_zMovePeriodTime * d_sspTimeStartR, d_zMoveAmplitude, d_zMoveAmplitudeShift);
    c_move_r = wsin(d_sspTimeEndR, d_aMovePeriodTime, d_aMovePhaseShift + 2 * M_PI / d_aMovePeriodTime * d_sspTimeStartR + M_PI, -d_aMoveAmplitude, -d_aMoveAmplitudeShift);
    pelvis_offset_l = 0;
    pelvis_offset_r = 0;
  }

  a_move_l = 0;
  b_move_l = 0;
  a_move_r = 0;
  b_move_r = 0;

  double ep[12];
  ep[0] = x_swap + x_move_r + d_xOffset;
  ep[1] = y_swap + y_move_r - d_yOffset / 2;
  ep[2] = z_swap + z_move_r + d_zOffset;
  ep[3] = a_swap + a_move_r - d_rOffset / 2;
  ep[4] = b_swap + b_move_r + d_pOffset;
  ep[5] = c_swap + c_move_r - d_aOffset / 2;
  ep[6] = x_swap + x_move_l + d_xOffset;
  ep[7] = y_swap + y_move_l + d_yOffset / 2;
  ep[8] = z_swap + z_move_l + d_zOffset;
  ep[9] = a_swap + a_move_l + d_rOffset / 2;
  ep[10] = b_swap + b_move_l + d_pOffset;
  ep[11] = c_swap + c_move_l + d_aOffset / 2;

  // Compute body swing
  if (d_time <= d_sspTimeEndL)
  {
    d_bodySwingY = -ep[7];
    d_bodySwingZ = ep[8];
  }
  else
  {
    d_bodySwingY = -ep[1];
    d_bodySwingZ = ep[2];
  }
  d_bodySwingZ -= LEG_LENGTH;

  double angle[14];
  
  // Compute arm swing
  if (d_xMoveAmplitude == 0)
  {
    angle[12] = 0; // Right
    angle[13] = 0; // Left
  }
  else
  {
    angle[12] = wsin(d_time, d_periodTime, M_PI * 1.5, -d_xMoveAmplitude * d_armSwingGain, 0);
    angle[13] = wsin(d_time, d_periodTime, M_PI * 1.5,  d_xMoveAmplitude * d_armSwingGain, 0);
  }

  if (d_isRunning)
  {
    d_time += TIME_UNIT;
    if (d_time >= d_periodTime)
      d_time = 0;
  }

  // Compute angles
  if (computeIK(&angle[0], ep[0], ep[1], ep[2], ep[3], ep[4], ep[5]) != 1 ||
      computeIK(&angle[6], ep[6], ep[7], ep[8], ep[9], ep[10], ep[11]) != 1)
  {
    // Do not use angle;
    if (!d_isRunning)
      setCompletedFlag();
    return;
  }

  // Convert leg angles from radians to degrees (skip shoulders and head pan)
  for (int i = 0; i < 12; i++)
    angle[i] *= 180.0 / M_PI;

  //                     R_HIP_YAW, R_HIP_ROLL, R_HIP_PITCH, R_KNEE, R_ANKLE_PITCH, R_ANKLE_ROLL, L_HIP_YAW, L_HIP_ROLL, L_HIP_PITCH, L_KNEE, L_ANKLE_PITCH, L_ANKLE_ROLL, R_ARM_SWING, L_ARM_SWING
  int dir[14]          = {   -1,        -1,          1,         1,         -1,            1,          -1,        -1,         -1,         -1,         1,            1,           1,           -1      };
  double initAngle[14] = {   0.0,       0.0,        0.0,       0.0,        0.0,          0.0,         0.0,       0.0,        0.0,        0.0,       0.0,          0.0,       -48.345,       41.313    };
  
  // Compute motor value
  for (int i = 0; i < 14; i++)
  {
    double offset = (double)dir[i] * angle[i] * MX28::RATIO_DEGS2VALUE;
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
    auto hw = AgentState::get<HardwareState>();
    assert(hw);
    auto gryoRaw = hw->getCM730State()->gyroRaw;

    // TODO review the gyro axes labels
    // TODO need to balance these values around the midpoint, taking calibration into account
    double rlGyroErr = gryoRaw.x();
    double fbGyroErr = gryoRaw.y();

    d_outValue[1]  += (int)(dir[1] * rlGyroErr * BALANCE_HIP_ROLL_GAIN); // R_HIP_ROLL
    d_outValue[7]  += (int)(dir[7] * rlGyroErr * BALANCE_HIP_ROLL_GAIN); // L_HIP_ROLL

    d_outValue[3]  -= (int)(dir[3] * fbGyroErr * BALANCE_KNEE_GAIN); // R_KNEE
    d_outValue[9]  -= (int)(dir[9] * fbGyroErr * BALANCE_KNEE_GAIN); // L_KNEE

    d_outValue[4]  -= (int)(dir[4]  * fbGyroErr * BALANCE_ANKLE_PITCH_GAIN); // R_ANKLE_PITCH
    d_outValue[10] -= (int)(dir[10] * fbGyroErr * BALANCE_ANKLE_PITCH_GAIN); // L_ANKLE_PITCH

    d_outValue[5]  -= (int)(dir[5]  * rlGyroErr * BALANCE_ANKLE_ROLL_GAIN); // R_ANKLE_ROLL
    d_outValue[11] -= (int)(dir[11] * rlGyroErr * BALANCE_ANKLE_ROLL_GAIN); // L_ANKLE_ROLL
  }
  
  if (!d_isRunning)
    setCompletedFlag();
  
  // Ensure all values are within the valid range
  for (int i = 0; i < 14; ++i)
    d_outValue[i] = MX28::clampValue(d_outValue[i]);
}

void WalkModule::applyHead(shared_ptr<HeadSection> head)
{
  // Ensure we have our standard PID values
  head->visitJoints([this](shared_ptr<JointControl> joint){ joint->setPidGains(P_GAIN, I_GAIN, D_GAIN); });

  head->pan()->setAngle(A_MOVE_AMPLITUDE);
}

void WalkModule::applyArms(shared_ptr<ArmSection> arms)
{
  // Arms move with a low P value of 8
  arms->visitJoints([this](shared_ptr<JointControl> joint){ joint->setPGain(8); });

  arms->shoulderPitchRight()->setValue(d_outValue[12]);
  arms->shoulderPitchLeft()->setValue(d_outValue[13]);

  arms->shoulderRollRight()->setAngle(-17);
  arms->shoulderRollLeft()->setAngle(17);
  arms->elbowRight()->setAngle(29);
  arms->elbowLeft()->setAngle(-29);
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
