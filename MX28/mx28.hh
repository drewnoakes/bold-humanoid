#pragma once

#include <cmath>
#include <string>

namespace bold
{
  typedef unsigned char uchar;
  typedef unsigned short ushort;

  /** EEPROM and RAM p. 4 in MX28 Technical Specifications PDF
   * This enum enumerates the addresses. The list depends on the firmware version of the MX28.
   */
  enum class MX28Table : uchar
  {
    MODEL_NUMBER_L            = 0,
    MODEL_NUMBER_H            = 1,
    VERSION                   = 2,
    ID                        = 3,
    BAUD_RATE                 = 4,
    RETURN_DELAY_TIME         = 5,
    CW_ANGLE_LIMIT_L          = 6,
    CW_ANGLE_LIMIT_H          = 7,
    CCW_ANGLE_LIMIT_L         = 8,
    CCW_ANGLE_LIMIT_H         = 9,
    SYSTEM_DATA2              = 10,
    HIGH_LIMIT_TEMPERATURE    = 11,
    LOW_LIMIT_VOLTAGE         = 12,
    HIGH_LIMIT_VOLTAGE        = 13,
    MAX_TORQUE_L              = 14,
    MAX_TORQUE_H              = 15,
    RETURN_LEVEL              = 16,
    ALARM_LED                 = 17,
    ALARM_SHUTDOWN            = 18,
    OPERATING_MODE            = 19,
    LOW_CALIBRATION_L         = 20,
    LOW_CALIBRATION_H         = 21,
    HIGH_CALIBRATION_L        = 22,
    HIGH_CALIBRATION_H        = 23,
    TORQUE_ENABLE             = 24,
    LED                       = 25,
    D_GAIN                    = 26,
    I_GAIN                    = 27,
    P_GAIN                    = 28,
    RESERVED                  = 29,
    GOAL_POSITION_L           = 30,
    GOAL_POSITION_H           = 31,
    MOVING_SPEED_L            = 32,
    MOVING_SPEED_H            = 33,
    TORQUE_LIMIT_L            = 34,
    TORQUE_LIMIT_H            = 35,
    PRESENT_POSITION_L        = 36,
    PRESENT_POSITION_H        = 37,
    PRESENT_SPEED_L           = 38,
    PRESENT_SPEED_H           = 39,
    PRESENT_LOAD_L            = 40,
    PRESENT_LOAD_H            = 41,
    PRESENT_VOLTAGE           = 42,
    PRESENT_TEMPERATURE       = 43,
    REGISTERED_INSTRUCTION    = 44,
    PAUSE_TIME                = 45,
    MOVING                    = 46,
    LOCK                      = 47,
    PUNCH_L                   = 48,
    PUNCH_H                   = 49,
    RESERVED4                 = 50,
    RESERVED5                 = 51,
    POT_L                     = 52,
    POT_H                     = 53,
    PWM_OUT_L                 = 54,
    PWM_OUT_H                 = 55,
    P_ERROR_L                 = 56,
    P_ERROR_H                 = 57,
    I_ERROR_L                 = 58,
    I_ERROR_H                 = 59,
    D_ERROR_L                 = 60,
    D_ERROR_H                 = 61,
    P_ERROR_OUT_L             = 62,
    P_ERROR_OUT_H             = 63,
    I_ERROR_OUT_L             = 64,
    I_ERROR_OUT_H             = 65,
    D_ERROR_OUT_L             = 66,
    D_ERROR_OUT_H             = 67,
    MAXNUM_ADDRESS
  };

  inline MX28Table operator-(MX28Table const& a, MX28Table const& b)
  {
    return (MX28Table)((uchar)a - (uchar)b);
  }

  /** Helper functions and constants related to the XM28 servo motor. */
  class MX28
  {
  public:
    static constexpr ushort MIN_VALUE    = 0x0000;
    static constexpr ushort CENTER_VALUE = 0x0800; // 2048
    static constexpr ushort MAX_VALUE    = 0x0FFF; // 4095

    static constexpr ushort MAX_TORQUE = 0x3FF; // 1023

    static constexpr double MIN_DEGS = -180.0;
    static constexpr double MAX_DEGS =  180.0;
    static constexpr double MIN_RADS = -M_PI;
    static constexpr double MAX_RADS =  M_PI;

    static constexpr double RATIO_VALUE2DEGS =  360.0 / 4096.0;
    static constexpr double RATIO_DEGS2VALUE = 4096.0 / 360.0;

    static constexpr double RATIO_VALUE2RADS = (2*M_PI) / 4096.0;
    static constexpr double RATIO_RADS2VALUE = 4096.0 / (2*M_PI);

    static constexpr double RATIO_VALUE2RPM = 0.053; // TODO express this with more precision
    static constexpr double RATIO_RPM2VALUE = 18.87; // TODO express this with more precision

    static constexpr double RATIO_VALUE2TORQUE = 0.01;
    static constexpr double RATIO_TORQUE2VALUE = 100;

    static ushort getMirrorValue(ushort value);
    static double getMirrorAngle(double angle);

    static ushort clampValue(int value);

    static ushort degs2Value(double angle);
    static double value2Degs(ushort value);

    static ushort rads2Value(double angle);
    static double value2Rads(ushort value);

    static ushort rpm2Value(double speed);
    static double value2Rpm(ushort value);

    static ushort torque2Value(double speed);
    static double value2Torque(ushort value);

    static uchar centigrade2Value(int degreesCentigrade);
    static int value2Centigrade(uchar value);

    static uchar voltage2Value(double volts);
    static double value2Voltage(uchar value);

    static double value2Load(ushort value);

    static std::string getAddressName(MX28Table address);
  };
}
