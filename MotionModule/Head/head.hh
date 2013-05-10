#pragma once

#include <string.h>

#include "../motionmodule.hh"
#include "../../Math/math.hh"

class minIni;

namespace bold
{
  class HeadSection;
  class ArmSection;
  class LegSection;

  class Head : public MotionModule
  {
  private:
    static constexpr double EYE_TILT_OFFSET_ANGLE = 40.0; // degrees

    double d_limitLeft;
    double d_limitRight;
    double d_limitTop;
    double d_limitBottom;

    double d_panHome;
    double d_tiltHome;

    double d_panGainP;
    double d_panGainD;
    double d_tiltGainP;
    double d_tiltGainD;

    double d_panError;
    double d_tiltError;

    double d_panAngle;
    double d_tiltAngle;

    void checkLimit();

  public:
    Head(minIni const& ini);
    ~Head();

    void initialize() override;
    void step(JointSelection const& selectedJoints) override;
    void applyHead(std::shared_ptr<HeadSection> head) override;
    void applyArms(std::shared_ptr<ArmSection> arms) override;
    void applyLegs(std::shared_ptr<LegSection> legs) override;

    // TODO don't store this here, but rather some static model of the body's limits

    double getTopLimitDegs() const    { return d_limitTop; }
    double getBottomLimitDegs() const { return d_limitBottom; }
    double getRightLimitDegs() const  { return d_limitRight; }
    double getLeftLimitDegs() const   { return d_limitLeft; }

    double getTopLimitRads() const    { return Math::degToRad(d_limitTop); }
    double getBottomLimitRads() const { return Math::degToRad(d_limitBottom); }
    double getRightLimitRads() const  { return Math::degToRad(d_limitRight); }
    double getLeftLimitRads() const   { return Math::degToRad(d_limitLeft); }

    double getPanDegs() const  { return d_panAngle; }
    double getTiltDegs() const { return d_tiltAngle; }

    double getPanRads() const  { return Math::degToRad(d_panAngle); }
    double getTiltRads() const { return Math::degToRad(d_tiltAngle); }

    /// Move the head to the position set as the 'home' position
    void moveToHome();

    // TODO create moveToDegs and moveToRads, etc

    /// Move to the absolute angular position specified
    void moveToAngle(double panDegs, double tiltDegs);

    /// Move the head by the delta specified
    void moveByAngleOffset(double panDegsDelta, double tiltDegsDelta);

    /// Reset motion tracking state to zero
    void initTracking();

    /// Use the provided error as the input into a PD controller
    void moveTracking(double panError, double tiltError);
  };
}
