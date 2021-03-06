#include "walkengine.hh"

#include "../../BodyControl/bodycontrol.hh"
#include "../../Config/config.hh"
#include "../../MX28/mx28.hh"
#include "../../State/state.hh"
#include "../../StateObject/HardwareState/hardwarestate.hh"

// TODO reimplement using Eigen and get rid of these classes:
#include "Vector.h"
#include "Matrix.h"

using namespace bold;
using namespace std;

WalkEngine::WalkEngine()
{
  X_OFFSET          = Config::getSetting<double>("walk-engine.params.x-offset");
  Y_OFFSET          = Config::getSetting<double>("walk-engine.params.y-offset");
  Z_OFFSET          = Config::getSetting<double>("walk-engine.params.z-offset");
  ROLL_OFFSET       = Config::getSetting<double>("walk-engine.params.roll-offset");
  PITCH_OFFSET      = Config::getSetting<double>("walk-engine.params.pitch-offset");
  YAW_OFFSET        = Config::getSetting<double>("walk-engine.params.yaw-offset");
  PERIOD_TIME       = Config::getSetting<double>("walk-engine.params.period-time");
  DSP_RATIO         = Config::getSetting<double>("walk-engine.params.dsp-ratio");
  STEP_FB_RATIO     = Config::getSetting<double>("walk-engine.params.step-fb-ratio");
  Z_MOVE_AMPLITUDE  = Config::getSetting<double>("walk-engine.params.foot-height");
  Y_SWAP_AMPLITUDE  = Config::getSetting<double>("walk-engine.params.swing-right-left");
  Z_SWAP_AMPLITUDE  = Config::getSetting<double>("walk-engine.params.swing-top-down");
  PELVIS_OFFSET     = Config::getSetting<double>("walk-engine.params.pelvis-offset");
  ARM_SWING_GAIN    = Config::getSetting<double>("walk-engine.params.arm-swing-gain");

  d_legGainP = Config::getSetting<int>("walk-engine.gains.leg-p-gain");
  d_legGainI = Config::getSetting<int>("walk-engine.gains.leg-i-gain");
  d_legGainD = Config::getSetting<int>("walk-engine.gains.leg-d-gain");
  d_armGainP = Config::getSetting<int>("walk-engine.gains.arm-p-gain");
  d_armGainI = Config::getSetting<int>("walk-engine.gains.arm-i-gain");
  d_armGainD = Config::getSetting<int>("walk-engine.gains.arm-d-gain");
  d_headGainP = Config::getSetting<int>("walk-engine.gains.head-p-gain");
  d_headGainI = Config::getSetting<int>("walk-engine.gains.head-i-gain");
  d_headGainD = Config::getSetting<int>("walk-engine.gains.head-d-gain");

  // Initialise dynamic control variables to sensible initial values
  HIP_PITCH_OFFSET = 13.0;
  X_MOVE_AMPLITUDE = 0;
  Y_MOVE_AMPLITUDE = 0;
  A_MOVE_AMPLITUDE = 0;
}

constexpr double WalkEngine::THIGH_LENGTH;
constexpr double WalkEngine::CALF_LENGTH;
constexpr double WalkEngine::ANKLE_LENGTH;
constexpr double WalkEngine::LEG_LENGTH;

//                                0           1           2         3           4             5           6           7           8           9          10           11            12            13
//                            R_HIP_YAW, R_HIP_ROLL, R_HIP_PITCH, R_KNEE, R_ANKLE_PITCH, R_ANKLE_ROLL, L_HIP_YAW, L_HIP_ROLL, L_HIP_PITCH, L_KNEE, L_ANKLE_PITCH, L_ANKLE_ROLL, R_ARM_SWING, L_ARM_SWING
const int dir[14]          = {   -1,        -1,          1,         1,         -1,            1,          -1,        -1,         -1,         -1,         1,            1,           1,           -1      };
const double initAngle[14] = {   0.0,       0.0,        0.0,       0.0,        0.0,          0.0,         0.0,       0.0,        0.0,        0.0,       0.0,          0.0,       -48.345,       41.313    };

double WalkEngine::wsin(double time, double period, double period_shift, double mag, double mag_shift)
{
  return mag * sin(2 * M_PI / period * time - period_shift) + mag_shift;
}

bool WalkEngine::computeIK(double *out, double x, double y, double z, double a, double b, double c)
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

void WalkEngine::updateTimeParams()
{
  d_periodTime = PERIOD_TIME->getValue();
  d_dspRatio = DSP_RATIO->getValue();
  d_sspRatio = 1 - d_dspRatio;

  d_xSwapPeriodTime = d_periodTime / 2;
  d_xMovePeriodTime = d_periodTime * d_sspRatio;
  d_ySwapPeriodTime = d_periodTime;
  d_yMovePeriodTime = d_periodTime * d_sspRatio;
  d_zSwapPeriodTime = d_periodTime / 2;
  d_zMovePeriodTime = d_periodTime * d_sspRatio / 2;
  d_aMovePeriodTime = d_periodTime * d_sspRatio;

  d_sspTimeStartL = (1 - d_sspRatio) * d_periodTime / 4;
  d_sspTimeEndL   = (1 + d_sspRatio) * d_periodTime / 4;
  d_sspTimeStartR = (3 - d_sspRatio) * d_periodTime / 4;
  d_sspTimeEndR   = (3 + d_sspRatio) * d_periodTime / 4;

  d_phaseTime1 = (d_sspTimeEndL + d_sspTimeStartL) / 2;
  d_phaseTime2 = (d_sspTimeStartR + d_sspTimeEndL) / 2;
  d_phaseTime3 = (d_sspTimeEndR + d_sspTimeStartR) / 2;

  d_pelvisOffset = PELVIS_OFFSET->getValue()*MX28::RATIO_DEGS2VALUE;
  d_pelvisSwing = d_pelvisOffset * 0.35;
  d_armSwingGain = ARM_SWING_GAIN->getValue();
}

void WalkEngine::updateMovementParams()
{
  // Forward/Back
  d_xMoveAmplitude = X_MOVE_AMPLITUDE;
  d_xSwapAmplitude = X_MOVE_AMPLITUDE * STEP_FB_RATIO->getValue();

  // Right/Left
  d_yMoveAmplitude = Y_MOVE_AMPLITUDE / 2;
  if (d_yMoveAmplitude > 0)
    d_yMoveAmplitudeShift = d_yMoveAmplitude;
  else
    d_yMoveAmplitudeShift = -d_yMoveAmplitude;
  d_ySwapAmplitude = Y_SWAP_AMPLITUDE->getValue() + d_yMoveAmplitudeShift * 0.04;

  d_zMoveAmplitude = Z_MOVE_AMPLITUDE->getValue() / 2;
  d_zMoveAmplitudeShift = d_zMoveAmplitude / 2;
  d_zSwapAmplitude = Z_SWAP_AMPLITUDE->getValue();
  d_zSwapAmplitudeShift = d_zSwapAmplitude;

  // Direction
  d_aMoveAmplitude = Math::degToRad(A_MOVE_AMPLITUDE) / 2.0;
  if (d_aMoveAmplitude > 0)
    d_aMoveAmplitudeShift = d_aMoveAmplitude;
  else
    d_aMoveAmplitudeShift = -d_aMoveAmplitude;
}

void WalkEngine::updateBalanceParams()
{
  d_xOffset = X_OFFSET->getValue();
  d_yOffset = Y_OFFSET->getValue();
  d_zOffset = Z_OFFSET->getValue();
  d_rOffset = Math::degToRad(ROLL_OFFSET->getValue());
  d_pOffset = Math::degToRad(PITCH_OFFSET->getValue());
  d_aOffset = Math::degToRad(YAW_OFFSET->getValue());
}

void WalkEngine::reset()
{
  log::info("WalkEngine::reset");

  X_MOVE_AMPLITUDE = 0;
  Y_MOVE_AMPLITUDE = 0;
  A_MOVE_AMPLITUDE = 0;
  HIP_PITCH_OFFSET = 13.0;

  d_bodySwingY = 0;
  d_bodySwingZ = 0;

  d_time = 0;
  updateTimeParams();
  updateMovementParams();

  // Copy the current pose to the output value array, in case the applyLegs etc
  // are called before a successful step completes.
  auto hw = State::get<HardwareState>();
  if (hw)
  {
    d_outValue[0]  = hw->getMX28State((uchar)JointId::R_HIP_YAW).presentPositionValue;
    d_outValue[1]  = hw->getMX28State((uchar)JointId::R_HIP_ROLL).presentPositionValue;
    d_outValue[2]  = hw->getMX28State((uchar)JointId::R_HIP_PITCH).presentPositionValue;
    d_outValue[3]  = hw->getMX28State((uchar)JointId::R_KNEE).presentPositionValue;
    d_outValue[4]  = hw->getMX28State((uchar)JointId::R_ANKLE_PITCH).presentPositionValue;
    d_outValue[5]  = hw->getMX28State((uchar)JointId::R_ANKLE_ROLL).presentPositionValue;
    d_outValue[6]  = hw->getMX28State((uchar)JointId::L_HIP_YAW).presentPositionValue;
    d_outValue[7]  = hw->getMX28State((uchar)JointId::L_HIP_ROLL).presentPositionValue;
    d_outValue[8]  = hw->getMX28State((uchar)JointId::L_HIP_PITCH).presentPositionValue;
    d_outValue[9]  = hw->getMX28State((uchar)JointId::L_KNEE).presentPositionValue;
    d_outValue[10] = hw->getMX28State((uchar)JointId::L_ANKLE_PITCH).presentPositionValue;
    d_outValue[11] = hw->getMX28State((uchar)JointId::L_ANKLE_ROLL).presentPositionValue;
    d_outValue[12] = hw->getMX28State((uchar)JointId::R_SHOULDER_PITCH).presentPositionValue;
    d_outValue[13] = hw->getMX28State((uchar)JointId::L_SHOULDER_PITCH).presentPositionValue;
  }
}

bool WalkEngine::canStopNow() const
{
  return d_canStopNow;
}

void WalkEngine::step()
{
  ASSERT(ThreadUtil::isMotionLoopThread());

  // Update walk parameters
  //
  // Param values are only copied at certain times during the gait to maintain
  // smooth behaviour.
  //
  bool canStop = false;

  if (d_time == 0)
  {
    updateTimeParams();
    d_phase = PHASE0;
    canStop = true; //d_xMoveAmplitude == 0 && d_yMoveAmplitude == 0 && d_aMoveAmplitude == 0;
  }
  else if (fabs(d_time - d_phaseTime1) <= TIME_UNIT/2)
  {
    updateMovementParams();
    d_phase = PHASE1;
  }
  else if (fabs(d_time - d_phaseTime2) <= TIME_UNIT/2)
  {
    updateTimeParams();
    d_time = d_phaseTime2;
    d_phase = PHASE2;
    canStop = true; //d_xMoveAmplitude == 0 && d_yMoveAmplitude == 0 && d_aMoveAmplitude == 0;
  }
  else if (fabs(d_time - d_phaseTime3) <= TIME_UNIT/2)
  {
    updateMovementParams();
    d_phase = PHASE3;
  }

  d_canStopNow = canStop;

  // Balance params are updated every step
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

  // Increment the time
  d_time += TIME_UNIT;
  if (d_time >= d_periodTime)
    d_time = 0;

  // Compute angles
  if (!computeIK(&angle[0], ep[0], ep[1], ep[2], ep[3], ep[4], ep[5]) ||
      !computeIK(&angle[6], ep[6], ep[7], ep[8], ep[9], ep[10], ep[11]))
  {
    // Do not use angles. Hinges will stay in the last position for which
    // values were successfully computed.
    log::error("WalkEngine::step") << "Error computing inverse kinematics";
    return;
  }

  // Convert leg angles from radians to degrees (skip shoulders and head pan)
  for (int i = 0; i < 12; i++)
    angle[i] *= 180.0 / M_PI;

  // Compute motor value
  for (int i = 0; i < 14; i++)
  {
    double offset = (double)dir[i] * angle[i] * MX28::RATIO_DEGS2VALUE;
    if (i == 1) // R_HIP_ROLL
      offset += (double)dir[i] * pelvis_offset_r;
    else if (i == 7) // L_HIP_ROLL
      offset += (double)dir[i] * pelvis_offset_l;
    else if (i == 2 || i == 8) // R_HIP_PITCH or L_HIP_PITCH
      offset -= (double)dir[i] * HIP_PITCH_OFFSET * MX28::RATIO_DEGS2VALUE; // NOTE we don't snapshot this parameter, and always use the most recent

    d_outValue[i] = MX28::degs2Value(initAngle[i]) + (int)offset;
  }
}

void WalkEngine::applyHead(HeadSection* head)
{
  // Ensure we have our standard PID values
  head->visitJoints([this](JointControl* joint) { joint->setPidGains((uchar)d_headGainP->getValue(), (uchar)d_headGainI->getValue(), (uchar)d_headGainD->getValue()); });

  // Head pan follows turn, if any
  head->pan()->setDegrees(A_MOVE_AMPLITUDE);
}

void WalkEngine::applyArms(ArmSection* arms)
{
  // Arms move with a low P value of 8
  arms->visitJoints([this](JointControl* joint) { joint->setPidGains((uchar)d_armGainP->getValue(), (uchar)d_armGainI->getValue(), (uchar)d_armGainD->getValue()); });

  arms->shoulderPitchRight()->setValue(MX28::clampValue(d_outValue[12]));
  arms->shoulderPitchLeft()->setValue(MX28::clampValue(d_outValue[13]));

  arms->shoulderRollRight()->setDegrees(-17);
  arms->shoulderRollLeft()->setDegrees(17);
  arms->elbowRight()->setDegrees(29);
  arms->elbowLeft()->setDegrees(-29);
}

void WalkEngine::applyLegs(LegSection* legs)
{
  // Ensure we have our standard PID values
  legs->visitJoints([this](JointControl* joint) { joint->setPidGains((uchar)d_legGainP->getValue(), (uchar)d_legGainI->getValue(), (uchar)d_legGainD->getValue()); });

  legs->hipYawRight()->setValue(MX28::clampValue(d_outValue[0]));
  legs->hipRollRight()->setValue(MX28::clampValue(d_outValue[1]));
  legs->hipPitchRight()->setValue(MX28::clampValue(d_outValue[2]));
  legs->kneeRight()->setValue(MX28::clampValue(d_outValue[3]));
  legs->anklePitchRight()->setValue(MX28::clampValue(d_outValue[4]));
  legs->ankleRollRight()->setValue(MX28::clampValue(d_outValue[5]));
  legs->hipYawLeft()->setValue(MX28::clampValue(d_outValue[6]));
  legs->hipRollLeft()->setValue(MX28::clampValue(d_outValue[7]));
  legs->hipPitchLeft()->setValue(MX28::clampValue(d_outValue[8]));
  legs->kneeLeft()->setValue(MX28::clampValue(d_outValue[9]));
  legs->anklePitchLeft()->setValue(MX28::clampValue(d_outValue[10]));
  legs->ankleRollLeft()->setValue(MX28::clampValue(d_outValue[11]));
}
