#pragma once

#include <memory>
#include <string>
#include <Eigen/Core>

#include "../CM730Platform/cm730platform.hh"
#include "../Math/math.hh"
#include "../MX28/mx28.hh"

namespace bold
{
  class CM730Platform;

  typedef unsigned char uchar;

  class BulkReadTable
  {
  public:
    int startAddress;
    int length;
    uchar table[MX28::MAXNUM_ADDRESS];

    BulkReadTable();

    uchar readByte(uchar address) const;
    int readWord(uchar address) const;
  };

  class BulkRead
  {
  public:
    uchar error;
    unsigned deviceCount;
    int rxLength;
    BulkReadTable data[21];

    BulkRead(uchar cmMin, uchar cmMax, uchar mxMin, uchar mxMax);

    BulkReadTable const& getBulkReadData(uchar id) const;

    // TODO why is this const_cast needed?
    uchar* getTxPacket() const { return const_cast<uchar*>(&d_txPacket[0]); }

  private:
    uchar d_txPacket[5 + 1 + 3 + (20*3) + 1]; // 70
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
    /// Port error, failed to receive Status Packet
    RX_FAIL,
    /// Timeout Status, failed to receive Packet (check connection)
    RX_TIMEOUT,
    /// Status Packet error (bad communications link)
    RX_CORRUPT
  };

  class CM730
  {
  public:
    /// Error bit flags
    enum
    {
      /// Input Voltage range in over the limit.
      INPUT_VOLTAGE   = 1,
      /// Set Angle limit problem(s).
      ANGLE_LIMIT     = 2,
      /// Internal overheating.
      OVERHEATING     = 4,
      /// Set value(s) out of range.
      RANGE           = 8,
      /// Instruction Packet Checksum error.
      CHECKSUM        = 16,
      /// Excessive load detected.
      OVERLOAD        = 32,
      /// Invalid Instruction Packet Instruction.
      INSTRUCTION     = 64
    };

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
      P_LED_PANNEL        = 25, /// LED of back panel
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

    /// Special device IDs
    enum
    {
      /// ID for Sub Controller
      ID_CM           = 200,
      /// Communication with all connected devices
      ID_BROADCAST    = 254
    };

  public:
    static int makeWord(uchar lowByte, uchar highByte) { return (highByte << 8) | lowByte; }
    static uchar getLowByte(int word) { return word & 0xFF; }
    static uchar getHighByte(int word) { return (word >> 8) & 0xFF; }

    static int color2Value(uchar red, uchar green, uchar blue) { return (int)(((blue>>3)<<10)|((green>>3)<<5)|(red>>3)); }

    // 0 -> -1600 dps
    // 512 -> 0 dps
    // 1023 -> +1600 dps
    static constexpr double RATIO_VALUE2DPS =  1600.0               / 512.0;
    static constexpr double RATIO_VALUE2RPS = (1600.0*(M_PI/180.0)) / 512.0;
    static constexpr double RATIO_VALUE2GS = 4.0 / 512.0;

    static double gyroValueToDps(int value) { return (value - 512)*RATIO_VALUE2DPS; }
    static double gyroValueToRps(int value) { return (value - 512)*RATIO_VALUE2RPS; }
    static double accValueToGs(int value)   { return (value - 512)*RATIO_VALUE2GS; }

    static Eigen::Vector3d shortToColour(unsigned short s)
    {
      return Eigen::Vector3d(
        ( s        & 0x1F) / 31.0,
        ((s >> 5)  & 0x1F) / 31.0,
        ((s >> 10) & 0x1F) / 31.0);
    }

    bool DEBUG_PRINT;

    CM730(std::shared_ptr<CM730Platform> platform);
    ~CM730();

    static std::string getCommResultName(CommResult responseCode);
    static std::string getInstructionName(uchar instructionId);


    /// Links CM730, returning true on success.
    bool connect();

    bool changeBaud(int baud);

    /// Releases CM730.
    void disconnect();

    /// Turn on the CM730's Dynamixel power.
    bool dxlPowerOn();


    /// Check the existance of Dynamixel with selected id. Returns communication result enum value.
    CommResult ping(uchar id, uchar *error);


    /// Reads a byte from the CM730 control table. Returns communication result enum value.
    CommResult readByte(uchar id, int address, uchar *pValue, uchar *error);

    /// Reads two bytes from the CM730 control table. Returns communication result enum value.
    CommResult readWord(uchar id, int address, int *pValue, uchar *error);

    /// Reads a consecutive range of bytes from the CM730 control table. Returns communication result enum value.
    CommResult readTable(uchar id, int start_addr, int end_addr, uchar *table, uchar *error);


    /// Writes a byte into the control table for the specified Dynamixel device. Returns communication result enum value.
    CommResult writeByte(uchar id, int address, int value, uchar *error);

    /// Writes two bytes into the control table for the specified Dynamixel device. Returns communication result enum value.
    CommResult writeWord(uchar id, int address, int value, uchar *error);

    /** Simultaneously write data to several Dynamixels at a time. Useful for motion control.
     *
     * This command is used to control several Dynamixels with one Instruction Packet transmission.
     * When this command is used, several commands are transmitted at once, reducing communication overhead.
     * The start address in the control table and the number of values to write is the same across all
     * devices, though the values may differ by device.
     * SyncWrite is a broadcast message, so no Status packet is expected in response, hence no error code.
     *
     * @param start_addr starting address within the control table
     * @param each_length number of consecutive values to read from the control table, inclusive
     * @param deviceCount the number of Dynamixel devices to write to
     * @param pParam parameters to be written, of length (number*each_length)
     */
    CommResult syncWrite(int start_addr, int each_length, int deviceCount, int *pParam);


    /// Restores the state of the specified Dynamixel to the factory default setting.
    CommResult reset(uchar id);


    CommResult bulkRead(std::shared_ptr<BulkRead> bulkRead);


    unsigned long getReceivedByteCount() const { return d_platform->getReceivedByteCount(); }
    unsigned long getTransmittedByteCount() const { return d_platform->getTransmittedByteCount(); }
    void resetByteCounts() { d_platform->resetByteCounts(); }

  private:
    static const int NUMBER_OF_JOINTS = 20;

    std::shared_ptr<CM730Platform> d_platform;
    uchar d_controlTable[MAXNUM_ADDRESS];

    /**
     * @param priority select the queue for this exchange: 0=high 1=med 2=low
     */
    CommResult txRxPacket(uchar *txpacket, uchar *rxpacket, int priority, std::shared_ptr<BulkRead> bulkRead);

    static uchar calculateChecksum(uchar *packet);    
  };
}
