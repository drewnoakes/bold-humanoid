#include "orientationtracker.hh"

#include "../../CM730Snapshot/cm730snapshot.hh"
#include "../../Config/config.hh"
#include "../../State/state.hh"
#include "../../StateObject/OrientationState/orientationstate.hh"
#include "../../util/assert.hh"
#include "../../util/log.hh"

#include <math.h>

using namespace bold;
using namespace std;
using namespace Eigen;

// sampling period in seconds
#define deltat 0.008f

// gyroscope measurement error in rad/s (shown as 5 deg/s)
#define gyroMeasError 3.14159265358979f * (5.0f / 180.0f)

// compute beta
#define beta sqrt(3.0f / 4.0f) * gyroMeasError

OrientationTracker::OrientationTracker()
: TypedStateObserver<HardwareState>("Orientation tracker", ThreadId::MotionLoop),
  d_technique(Config::getSetting<OrientationTechnique>("orientation-tracker.technique"))
{
  reset();
  Config::addAction("orientation-tracker.zero", "Zero", [this]() { reset(); });
}

void OrientationTracker::reset()
{
  SEq_1 = 1.0f;
  SEq_2 = 0.0f;
  SEq_3 = 0.0f;
  SEq_4 = 0.0f;
}

Quaterniond OrientationTracker::getQuaternion() const
{
  return Quaterniond(SEq_1, SEq_2, SEq_3, SEq_4);
}

void OrientationTracker::observeTyped(shared_ptr<HardwareState const> const& state, SequentialTimer& timer)
{
  switch (d_technique->getValue())
  {
    case OrientationTechnique::Madgwick:
      updateMadgwick(state);
      break;

    case OrientationTechnique::Sum:
      updateSum(state);
      break;

    default:
      log::error("OrientationTracker::observeTyped") << "Unexpected OrientationTechnique value: " << (int)d_technique->getValue();
      throw runtime_error("Unexpected OrientationTechnique value");
  }

  State::make<OrientationState>(getQuaternion());
}

void OrientationTracker::updateMadgwick(shared_ptr<HardwareState const> const& state)
{
  //
  // Using technique from paper:
  //
  //   "An efficient orientation filter for inertial and inertial/magnetic sensor arrays"
  //
  //   Sebastian O.H. Madgwick, April 30, 2010
  //

  Vector3d const& gyro(state->getCM730State().gyro);
  Vector3d const& acc(state->getCM730State().acc);

  float w_x = gyro.x();
  float w_y = gyro.y();
  float w_z = gyro.z();
  float a_x = acc.x();
  float a_y = acc.y();
  float a_z = acc.z();

  //
  // Auxiliary variables to avoid repeated calculations
  //

  float halfSEq_1 = 0.5f * SEq_1;
  float halfSEq_2 = 0.5f * SEq_2;
  float halfSEq_3 = 0.5f * SEq_3;
  float halfSEq_4 = 0.5f * SEq_4;
  float twoSEq_1 = 2.0f * SEq_1;
  float twoSEq_2 = 2.0f * SEq_2;
  float twoSEq_3 = 2.0f * SEq_3;

  // Normalise the accelerometer measurement
  float norm = sqrt(a_x * a_x + a_y * a_y + a_z * a_z);
  a_x /= norm;
  a_y /= norm;
  a_z /= norm;

  // Compute the objective function

  float f_1 = twoSEq_2 * SEq_4 - twoSEq_1 * SEq_3 - a_x;
  float f_2 = twoSEq_1 * SEq_2 + twoSEq_3 * SEq_4 - a_y;
  float f_3 = 1.0f - twoSEq_2 * SEq_2 - twoSEq_3 * SEq_3 - a_z;

  // Compute the Jacobian

  // J_11 negated in matrix multiplication
  float J_11or24 = twoSEq_3;
  float J_12or23 = 2.0f * SEq_4;
  // J_12 negated in matrix multiplication
  float J_13or22 = twoSEq_1;
  float J_14or21 = twoSEq_2;
  // negated in matrix multiplication
  float J_32 = 2.0f * J_14or21;
  // negated in matrix multiplication
  float J_33 = 2.0f * J_11or24;

  // Compute the gradient (matrix multiplication)
  // (estimated direction of the gyroscope error)
  float SEqHatDot_1 = J_14or21 * f_2 - J_11or24 * f_1;
  float SEqHatDot_2 = J_12or23 * f_1 + J_13or22 * f_2 - J_32 * f_3;
  float SEqHatDot_3 = J_12or23 * f_2 - J_33 * f_3 - J_13or22 * f_1;
  float SEqHatDot_4 = J_14or21 * f_1 + J_11or24 * f_2;

  // Normalise the gradient
  norm = sqrt(SEqHatDot_1 * SEqHatDot_1 + SEqHatDot_2 * SEqHatDot_2 + SEqHatDot_3 * SEqHatDot_3 + SEqHatDot_4 * SEqHatDot_4);
  SEqHatDot_1 /= norm;
  SEqHatDot_2 /= norm;
  SEqHatDot_3 /= norm;
  SEqHatDot_4 /= norm;

  // Compute the quaternion derivative measured by gyroscopes
  float SEqDot_omega_1 = -halfSEq_2 * w_x - halfSEq_3 * w_y - halfSEq_4 * w_z;
  float SEqDot_omega_2 = halfSEq_1 * w_x + halfSEq_3 * w_z - halfSEq_4 * w_y;
  float SEqDot_omega_3 = halfSEq_1 * w_y - halfSEq_2 * w_z + halfSEq_4 * w_x;
  float SEqDot_omega_4 = halfSEq_1 * w_z + halfSEq_2 * w_y - halfSEq_3 * w_x;

  // Compute then integrate the estimated quaternion derivative
  SEq_1 += (SEqDot_omega_1 - (beta * SEqHatDot_1)) * deltat;
  SEq_2 += (SEqDot_omega_2 - (beta * SEqHatDot_2)) * deltat;
  SEq_3 += (SEqDot_omega_3 - (beta * SEqHatDot_3)) * deltat;
  SEq_4 += (SEqDot_omega_4 - (beta * SEqHatDot_4)) * deltat;

  // Normalise quaternion
  norm = sqrt(SEq_1 * SEq_1 + SEq_2 * SEq_2 + SEq_3 * SEq_3 + SEq_4 * SEq_4);
  SEq_1 /= norm;
  SEq_2 /= norm;
  SEq_3 /= norm;
  SEq_4 /= norm;
}

void OrientationTracker::updateSum(shared_ptr<HardwareState const> const& state)
{
  Quaternionf curr(SEq_1, SEq_2, SEq_3, SEq_4);
  Vector3d const& gyro(state->getCM730State().gyro);
  Matrix3f m;
  m = AngleAxisf((float)Math::degToRad(gyro.x()), Vector3f::UnitX())
    * AngleAxisf((float)Math::degToRad(gyro.y()), Vector3f::UnitY())
    * AngleAxisf((float)Math::degToRad(gyro.z()), Vector3f::UnitZ());

  curr = curr * m;

  SEq_1 = curr.w();
  SEq_2 = curr.x();
  SEq_3 = curr.y();
  SEq_4 = curr.z();

  // Normalise quaternion
  float norm = sqrt(SEq_1 * SEq_1 + SEq_2 * SEq_2 + SEq_3 * SEq_3 + SEq_4 * SEq_4);
  SEq_1 /= norm;
  SEq_2 /= norm;
  SEq_3 /= norm;
  SEq_4 /= norm;
}
