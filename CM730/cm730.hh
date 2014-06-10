#pragma once

#include <memory>
#include <string>
#include <Eigen/Core>

#include "../CM730Platform/cm730platform.hh"
#include "../JointId/jointid.hh"
#include "../Math/math.hh"
#include "../MX28/mx28.hh"
#include "../MX28Alarm/mx28alarm.hh"

namespace bold
{
  typedef unsigned char uchar;

  namespace instruction
  {
    constexpr uchar Ping = 1;
    constexpr uchar Read = 2;
    constexpr uchar Write = 3;
    constexpr uchar RegWrite = 4;
    constexpr uchar Action = 5;
    constexpr uchar Reset = 6;
    constexpr uchar SyncWrite = 131; // 0x83
    constexpr uchar BulkRead = 146; // 0x92
  }

  class BulkReadTable
  {
  public:
    BulkReadTable();

    uchar readByte(uchar address) const;
    ushort readWord(uchar address) const;
    uchar getStartAddress() const { return d_startAddress; }
    void setStartAddress(uchar address) { d_startAddress = address; }
    uint getLength() const { return d_length; }
    void setLength(uint length) { d_length = length; }
    uchar* getData() { return d_table.data(); }

  private:
    uchar d_startAddress;
    uint d_length;
    std::array<uchar,MX28::MAXNUM_ADDRESS> d_table;
  };

  class BulkRead
  {
  public:
    BulkRead(uchar cmMin, uchar cmMax, uchar mxMin, uchar mxMax);

    BulkReadTable& getBulkReadData(uchar id);
    uchar* getTxPacket() { return d_txPacket.data(); }
    void clearError() { d_error = (uchar)-1; }
    void setError(uchar error) { d_error = error; }
    uchar getError() const { return d_error; }
    uint getRxLength() const { return d_rxLength; }

  private:
    /// Position 0 for the CM730, and the rest for MX28s
    std::array<BulkReadTable, 1 + (uchar)JointId::DEVICE_COUNT> d_data;               // 21
    std::array<uchar, 5 + 1 + 3 + ((uchar)JointId::DEVICE_COUNT * 3) + 1> d_txPacket; // 70
    uint d_rxLength;
    // TODO is this an MX28Alarm?
    uchar d_error;
  };

  /// Communication results
  enum class CommResult
  {
    /// Successful communication with Dynamixel
    SUCCESS,
    /// Problems with Instruction Packet
    TX_CORRUPT,
    /// Port error, failed to send Instruction Packet
    TX_FAIL,
    /// Timeout Status, failed to receive Packet (check connection)
    RX_TIMEOUT,
    /// Status Packet error (bad communications link)
    RX_CORRUPT
  };

  class CM730
  {
  public:
    /** Identifies a value in EEPROM or RAM.
     * See page 4 in MX28 Technical Specifications PDF for more information.
     */
    enum
    {
      P_MODEL_NUMBER_L    = 0, /// Lowest byte of model number
      P_MODEL_NUMBER_H    = 1, /// Highest byte of model number
      P_VERSION           = 2, /// Information on the version of firmware
      P_ID                = 3, /// ID of CM730
      P_BAUD_RATE         = 4, /// Baud Rate of CM730
      P_RETURN_DELAY_TIME = 5, /// Return Delay Time
      P_RETURN_LEVEL      = 16, /// Status Return Level
      P_DXL_POWER         = 24, /// Dynamixel Power
      P_LED_PANEL         = 25, /// LED of back panel
      P_LED_HEAD_L        = 26, /// Low byte of Head LED
      P_LED_HEAD_H        = 27, /// High byte of Head LED
      P_LED_EYE_L         = 28, /// Low byte of Eye LED
      P_LED_EYE_H         = 29, /// High byte of Eye LED
      P_BUTTON            = 30, /// Button
      P_GYRO_Z_L          = 38, /// Low byte of Gyro Z-axis
      P_GYRO_Z_H          = 39, /// High byte of Gyro Z-axis
      P_GYRO_Y_L          = 40, /// Low byte of Gyro Y-axis
      P_GYRO_Y_H          = 41, /// High byte of Gyro Y-axis
      P_GYRO_X_L          = 42, /// Low byte of Gyro X-axis
      P_GYRO_X_H          = 43, /// High byte of Gyro X-axis
      P_ACCEL_X_L         = 44, /// Low byte of Accelerometer X-axis
      P_ACCEL_X_H         = 45, /// High byte of Accelerometer X-axis
      P_ACCEL_Y_L         = 46, /// Low byte of Accelerometer Y-axis
      P_ACCEL_Y_H         = 47, /// High byte of Accelerometer Y-axis
      P_ACCEL_Z_L         = 48, /// Low byte of Accelerometer Z-axis
      P_ACCEL_Z_H         = 49, /// High byte of Accelerometer Z-axis
      P_VOLTAGE           = 50, /// Present Voltage
      P_LEFT_MIC_L        = 51, /// Low byte of Left Mic. ADC value
      P_LEFT_MIC_H        = 52, /// High byte of Left Mic. ADC value
      P_ADC2_L            = 53, /// Low byte of ADC 2
      P_ADC2_H            = 54, /// High byte of ADC 2
      P_ADC3_L            = 55, /// Low byte of ADC 3
      P_ADC3_H            = 56, /// High byte of ADC 3
      P_ADC4_L            = 57, /// Low byte of ADC 4
      P_ADC4_H            = 58, /// High byte of ADC 4
      P_ADC5_L            = 59, /// Low byte of ADC 5
      P_ADC5_H            = 60, /// High byte of ADC 5
      P_ADC6_L            = 61, /// Low byte of ADC 6
      P_ADC6_H            = 62, /// High byte of ADC 6
      P_ADC7_L            = 63, /// Low byte of ADC 7
      P_ADC7_H            = 64, /// High byte of ADC 7
      P_ADC8_L            = 65, /// Low byte of ADC 8
      P_ADC8_H            = 66, /// High byte of ADC 8
      P_RIGHT_MIC_L       = 67, /// Low byte of Right Mic. ADC value
      P_RIGHT_MIC_H       = 68, /// High byte of Right Mic. ADC value
      P_ADC10_L           = 69, /// Low byte of ADC 9
      P_ADC10_H           = 70, /// High byte of ADC 9
      P_ADC11_L           = 71, /// Low byte of ADC 10
      P_ADC11_H           = 72, /// High byte of ADC 10
      P_ADC12_L           = 73, /// Low byte of ADC 11
      P_ADC12_H           = 74, /// High byte of ADC 11
      P_ADC13_L           = 75, /// Low byte of ADC 12
      P_ADC13_H           = 76, /// High byte of ADC 12
      P_ADC14_L           = 77, /// Low byte of ADC 13
      P_ADC14_H           = 78, /// High byte of ADC 13
      P_ADC15_L           = 79, /// Low byte of ADC 14
      P_ADC15_H           = 80, /// High byte of ADC 14
      MAXNUM_ADDRESS
    };

    /// ID of CM730 sub-controller
    static constexpr uchar ID_CM = 200;

    /// Specifies that communication is to all connected devices
    static constexpr uchar ID_BROADCAST = 254;

    //
    // ----------------- Static members
    //

    static constexpr ushort makeWord(uchar lowByte, uchar highByte) { return (highByte << 8) | lowByte; }
    static constexpr uchar getLowByte(ushort word) { return word & 0xFF; }
    static constexpr uchar getHighByte(ushort word) { return (word >> 8) & 0xFF; }

    static constexpr ushort color2Value(uchar red, uchar green, uchar blue) { return (ushort)(((blue>>3)<<10)|((green>>3)<<5)|(red>>3)); }

    // 0 -> -1600 dps
    // 512 -> 0 dps
    // 1023 -> +1600 dps
    static constexpr double RATIO_VALUE2DPS =                1600.0  / 512.0;
    static constexpr double RATIO_VALUE2RPS = Math::degToRad(1600.0) / 512.0;
    static constexpr double RATIO_VALUE2GS  = 4.0 / 512.0;
    static constexpr int ACC_VALUE_MID = 512;
    static constexpr int GYRO_VALUE_MID = 512;

    static ushort flipImuValue(ushort value)
    {
      ASSERT(value <= 1023);
      if (value == 0)
        return 1023;
      return 1023 - value + 1;
    }

    static constexpr double gyroValueToDps(ushort value) { return ((int)value - GYRO_VALUE_MID)*RATIO_VALUE2DPS; }
    static constexpr double gyroValueToRps(ushort value) { return ((int)value - GYRO_VALUE_MID)*RATIO_VALUE2RPS; }
    static constexpr double accValueToGs(ushort value)   { return ((int)value - ACC_VALUE_MID)*RATIO_VALUE2GS; }

    static Eigen::Vector3d shortToColour(ushort s)
    {
      return Eigen::Vector3d(
        ( s        & 0x1F) / 31.0,
        ((s >> 5)  & 0x1F) / 31.0,
        ((s >> 10) & 0x1F) / 31.0);
    }

    static std::string getCommResultName(CommResult responseCode);

    //
    // ----------------- Instance members
    //

    CM730(std::unique_ptr<CM730Platform> platform);
    ~CM730();

    /// Links CM730, returning true on success.
    bool connect();

    /// Releases CM730, returning true on success.
    bool disconnect();

    /// True if connected to the CM730.
    bool isConnected() const { return d_platform->isPortOpen(); }

    /// Changes the communication baud rate, returning true on success.
    bool changeBaud(unsigned baud);

    /// Enable or disable torque for all joints, returning true on success.
    bool torqueEnable(bool enable);

    /// Enable or disable the CM730 power, returning true on success.
    bool powerEnable(bool enable);

    /// Ask the CM730 if the power is enabled.
    bool isPowerEnabled();


    unsigned long getReceivedByteCount() const { return d_platform->getReceivedByteCount(); }
    unsigned long getTransmittedByteCount() const { return d_platform->getTransmittedByteCount(); }
    void resetByteCounts() { d_platform->resetByteCounts(); }


    /// Check the existence of Dynamixel with selected id. Returns communication result enum value.
    CommResult ping(uchar id, MX28Alarm* error);

    /// Restores the state of the specified Dynamixel to the factory default setting.
    CommResult reset(uchar id);


    /// Reads a byte from the CM730 control table. Returns communication result enum value.
    CommResult readByte(uchar id, uchar address, uchar *pValue, MX28Alarm* error);

    /// Reads two bytes from the CM730 control table. Returns communication result enum value.
    CommResult readWord(uchar id, uchar address, ushort *pValue, MX28Alarm* error);

    /// Reads a consecutive range of bytes from the CM730 control table. Returns communication result enum value.
    CommResult readTable(uchar id, uchar fromAddress, uchar toAddress, uchar *table, MX28Alarm* error);

    // Executes a bulk read operation, as specified in bulkRead. Returns communication result enum value.
    CommResult bulkRead(BulkRead* bulkRead);


    /// Writes a byte into the control table for the specified Dynamixel device. Returns communication result enum value.
    CommResult writeByte(uchar id, uchar address, uchar value, MX28Alarm* error);

    /// Writes two bytes into the control table for the specified Dynamixel device. Returns communication result enum value.
    CommResult writeWord(uchar id, uchar address, ushort value, MX28Alarm* error);

    /** Simultaneously write consecutive table values to one ore more devices.
     *
     * This command can be used to control several Dynamixels with one instruction packet transmission.
     * Similarly, it can be used to write multiple consecutive values to a single device, such as the CM730.
     * In combination, it may be used to write different values across the same addresses on multiple MX28s, which is very useful for motion control.
     *
     * When this command is used, several commands are transmitted at once, reducing communication overhead.
     * The start address in the control table and the number of values to write is the same across all
     * devices, though the values may differ by device.
     *
     * SyncWrite is a broadcast message, so no Status packet is expected in response, hence no error code.
     *
     * @param fromAddress starting address within the control table
     * @param bytesPerDevice number of consecutive values to read from the control table, inclusive
     * @param deviceCount the number of Dynamixel devices to write to
     * @param parameters parameters to be written, of length (number*bytesPerDevice)
     */
    CommResult syncWrite(uchar fromAddress, uchar bytesPerDevice, uchar deviceCount, uchar *parameters);

  private:
    std::unique_ptr<CM730Platform> d_platform;
    bool d_isPowerEnableRequested;

    /**
     * @param bulkRead populated if txpacket[INSTRUCTION] == INST_BULK_READ, otherwise nullptr.
     */
    CommResult txRxPacket(uchar* txpacket, uchar* rxpacket, BulkRead* bulkRead = nullptr);

    static uchar calculateChecksum(uchar *packet);
  };
}
