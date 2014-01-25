#pragma once

#include <string.h>

#include "../motionmodule.hh"
#include "../../Math/math.hh"
#include "../../Setting/setting.hh"
#include "../../util/Range.hh"

namespace bold
{
  class ArmSection;
  class HeadSection;
  class LegSection;

  class HeadModule : public MotionModule
  {
  public:
    HeadModule(std::shared_ptr<MotionTaskScheduler> scheduler);
    ~HeadModule() override;

    HeadModule(const HeadModule&) = delete;
    HeadModule& operator=(const HeadModule&) = delete;

    void initialize() override;
    void step(std::shared_ptr<JointSelection> selectedJoints) override;
    void applyHead(std::shared_ptr<HeadSection> head) override;
    void applyArms(std::shared_ptr<ArmSection> arms) override;
    void applyLegs(std::shared_ptr<LegSection> legs) override;

    // TODO don't store this here, but rather some static model of the body's limits

    double getTopLimitDegs() const    { return d_limitTiltDegs->getValue().max(); }
    double getBottomLimitDegs() const { return d_limitTiltDegs->getValue().min(); }
    double getRightLimitDegs() const  { return d_limitPanDegs->getValue().min(); }
    double getLeftLimitDegs() const   { return d_limitPanDegs->getValue().max(); }

    double getTopLimitRads() const    { return Math::degToRad(getTopLimitDegs()); }
    double getBottomLimitRads() const { return Math::degToRad(getBottomLimitDegs()); }
    double getRightLimitRads() const  { return Math::degToRad(getRightLimitDegs()); }
    double getLeftLimitRads() const   { return Math::degToRad(getLeftLimitDegs()); }

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

    Setting<Range<double>>* d_limitPanDegs;
    Setting<Range<double>>* d_limitTiltDegs;

    Setting<double>* d_panHomeDegs;
    Setting<double>* d_tiltHomeDegs;

    /// P gain value set on the MX28
    Setting<int>* d_gainP;

    Setting<double>* d_panGainP;  ///< P gain value for pan joint used in tracking calculations
    Setting<double>* d_panGainI;  ///< I gain value for pan joint used in tracking calculations
    Setting<double>* d_panILeak;  ///< I leak value for pan joint used in tracking calculations
    Setting<double>* d_panGainD;  ///< D gain value for pan joint used in tracking calculations
    Setting<double>* d_tiltGainP; ///< P gain value for tilt joint used in tracking calculations
    Setting<double>* d_tiltGainI; ///< I gain value for tilt joint used in tracking calculations
    Setting<double>* d_tiltILeak;  ///< I leak value for pan joint used in tracking calculations
    Setting<double>* d_tiltGainD; ///< D gain value for tilt joint used in tracking calculations

    double d_lastPanError;
    double d_lastTiltError;

    double d_integratedPanError;
    double d_integratedTiltError;

    double d_targetPanAngleDegs;
    double d_targetTiltAngleDegs;
  };
}
