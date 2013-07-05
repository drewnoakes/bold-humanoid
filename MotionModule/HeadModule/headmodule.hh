#pragma once

#include <string.h>

#include "../motionmodule.hh"
#include "../../Control/control.hh"
#include "../../Math/math.hh"

namespace bold
{
  class HeadSection;
  class ArmSection;
  class LegSection;

  class HeadModule : public MotionModule
  {
  public:
    HeadModule(std::shared_ptr<MotionTaskScheduler> scheduler);
    ~HeadModule();

    std::vector<std::shared_ptr<Control const>> getControls() const { return d_controls; }

    void initialize() override;
    void step(std::shared_ptr<JointSelection> selectedJoints) override;
    void applyHead(std::shared_ptr<HeadSection> head) override;
    void applyArms(std::shared_ptr<ArmSection> arms) override;
    void applyLegs(std::shared_ptr<LegSection> legs) override;

    // TODO don't store this here, but rather some static model of the body's limits

    double getTopLimitDegs() const    { return d_limitTopDegs; }
    double getBottomLimitDegs() const { return d_limitBottomDegs; }
    double getRightLimitDegs() const  { return d_limitRightDegs; }
    double getLeftLimitDegs() const   { return d_limitLeftDegs; }

    double getTopLimitRads() const    { return Math::degToRad(d_limitTopDegs); }
    double getBottomLimitRads() const { return Math::degToRad(d_limitBottomDegs); }
    double getRightLimitRads() const  { return Math::degToRad(d_limitRightDegs); }
    double getLeftLimitRads() const   { return Math::degToRad(d_limitLeftDegs); }

    double getPanDegs() const  { return d_panAngle; }
    double getTiltDegs() const { return d_tiltAngle; }

    double getPanRads() const  { return Math::degToRad(d_panAngle); }
    double getTiltRads() const { return Math::degToRad(d_tiltAngle); }

    /// Move the head to the position set as the 'home' position
    void moveToHome();

    /** Move to the absolute angular position specified.
     *
     * @param panDegsDelta zero center, positive left, negative right
     * @param tiltDegsDelta zero center, positive up, negative down
     */
    void moveToDegs(double panDegs, double tiltDegs);

    /** Move the head by the delta specified.
     *
     * @param panDegsDelta positive left, negative right
     * @param tiltDegsDelta positive up, negative down
     */
    void moveByDeltaDegs(double panDegsDelta, double tiltDegsDelta);

    /// Reset motion tracking state to zero
    void initTracking();

    /// Use the provided error as the input into a PD controller
    void moveTracking(double panError, double tiltError);

  private:
    void checkLimit();

    std::vector<std::shared_ptr<Control const>> d_controls;

    double d_limitLeftDegs;
    double d_limitRightDegs;
    double d_limitTopDegs;
    double d_limitBottomDegs;

    double d_panHomeDegs;
    double d_tiltHomeDegs;

    /// P gain value set on the MX28
    double d_gainP;

    double d_panGainP;  ///< P gain value for pan joint used in tracking calculations
    double d_panGainD;  ///< D gain value for pan joint used in tracking calculations
    double d_tiltGainP; ///< P gain value for tilt joint used in tracking calculations
    double d_tiltGainD; ///< D gain value for tilt joint used in tracking calculations

    double d_lastPanError;
    double d_lastTiltError;

    double d_panAngle;
    double d_tiltAngle;
  };
}
