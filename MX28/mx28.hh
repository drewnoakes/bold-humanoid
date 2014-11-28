#pragma once

#include <cmath>
#include <string>

namespace bold
{
  typedef unsigned char uchar;
  typedef unsigned short ushort;

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

    static std::string getAddressName(uchar address);

    /** EEPROM and RAM p. 4 in MX28 Technical Specifications PDF
     * This enum enumerates the addresses. The list depends on the firmware version of the MX28.
     */
    enum
    {
      P_MODEL_NUMBER_L            = 0,
      P_MODEL_NUMBER_H            = 1,
      P_VERSION                   = 2,
      P_ID                        = 3,
      P_BAUD_RATE                 = 4,
      P_RETURN_DELAY_TIME         = 5,
      P_CW_ANGLE_LIMIT_L          = 6,
      P_CW_ANGLE_LIMIT_H          = 7,
      P_CCW_ANGLE_LIMIT_L         = 8,
      P_CCW_ANGLE_LIMIT_H         = 9,
      P_SYSTEM_DATA2              = 10,
      P_HIGH_LIMIT_TEMPERATURE    = 11,
      P_LOW_LIMIT_VOLTAGE         = 12,
      P_HIGH_LIMIT_VOLTAGE        = 13,
      P_MAX_TORQUE_L              = 14,
      P_MAX_TORQUE_H              = 15,
      P_RETURN_LEVEL              = 16,
      P_ALARM_LED                 = 17,
      P_ALARM_SHUTDOWN            = 18,
      P_OPERATING_MODE            = 19,
      P_LOW_CALIBRATION_L         = 20,
      P_LOW_CALIBRATION_H         = 21,
      P_HIGH_CALIBRATION_L        = 22,
      P_HIGH_CALIBRATION_H        = 23,
      P_TORQUE_ENABLE             = 24,
      P_LED                       = 25,
      P_D_GAIN                    = 26,
      P_I_GAIN                    = 27,
      P_P_GAIN                    = 28,
      P_RESERVED                  = 29,
      P_GOAL_POSITION_L           = 30,
      P_GOAL_POSITION_H           = 31,
      P_MOVING_SPEED_L            = 32,
      P_MOVING_SPEED_H            = 33,
      P_TORQUE_LIMIT_L            = 34,
      P_TORQUE_LIMIT_H            = 35,
      P_PRESENT_POSITION_L        = 36,
      P_PRESENT_POSITION_H        = 37,
      P_PRESENT_SPEED_L           = 38,
      P_PRESENT_SPEED_H           = 39,
      P_PRESENT_LOAD_L            = 40,
      P_PRESENT_LOAD_H            = 41,
      P_PRESENT_VOLTAGE           = 42,
      P_PRESENT_TEMPERATURE       = 43,
      P_REGISTERED_INSTRUCTION    = 44,
      P_PAUSE_TIME                = 45,
      P_MOVING                    = 46,
      P_LOCK                      = 47,
      P_PUNCH_L                   = 48,
      P_PUNCH_H                   = 49,
      P_RESERVED4                 = 50,
      P_RESERVED5                 = 51,
      P_POT_L                     = 52,
      P_POT_H                     = 53,
      P_PWM_OUT_L                 = 54,
      P_PWM_OUT_H                 = 55,
      P_P_ERROR_L                 = 56,
      P_P_ERROR_H                 = 57,
      P_I_ERROR_L                 = 58,
      P_I_ERROR_H                 = 59,
      P_D_ERROR_L                 = 60,
      P_D_ERROR_H                 = 61,
      P_P_ERROR_OUT_L             = 62,
      P_P_ERROR_OUT_H             = 63,
      P_I_ERROR_OUT_L             = 64,
      P_I_ERROR_OUT_H             = 65,
      P_D_ERROR_OUT_L             = 66,
      P_D_ERROR_OUT_H             = 67,
      MAXNUM_ADDRESS
    };
  };
}
