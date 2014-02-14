#include "cm730.hh"

#include <cassert>
#include <iomanip>

#include "../JointId/jointid.hh"
#include "../ThreadUtil/threadutil.hh"
#include "../util/ccolor.hh"
#include "../util/log.hh"

using namespace bold;
using namespace std;

// TODO get rid of these #defines

#define ID                 (2)
#define LENGTH             (3)
#define INSTRUCTION        (4)
#define ERRBIT             (4)
#define PARAMETER          (5)
#define DEFAULT_BAUDNUMBER (1)

#define INST_PING          (1)
#define INST_READ          (2)
#define INST_WRITE         (3)
#define INST_REG_WRITE     (4)
#define INST_ACTION        (5)
#define INST_RESET         (6)
#define INST_SYNC_WRITE    (131)   // 0x83
#define INST_BULK_READ     (146)   // 0x92

#define DEBUG_PRINT        (false)

//////////
////////// BulkRead
//////////

BulkRead::BulkRead(uchar cmMin, uchar cmMax, uchar mxMin, uchar mxMax)
: error(-1),
  deviceCount(1 + 20)
{
  // Build a place for the data we read back
  for (int i = 0; i < 21; i++)
    data[i] = BulkReadTable();

  // Create a cached TX packet as it'll be identical each time
  d_txPacket[ID]          = (uchar)CM730::ID_BROADCAST;
  d_txPacket[INSTRUCTION] = INST_BULK_READ;

  uchar p = PARAMETER;

  d_txPacket[p++] = (uchar)0x0;

  rxLength = deviceCount * 6;

  auto writeDeviceRequest = [&p,this](uchar deviceId, uchar startAddress, uchar endAddress)
  {
    assert(startAddress < endAddress);

    uchar requestedByteCount = endAddress - startAddress + 1;

    rxLength += requestedByteCount;

    d_txPacket[p++] = requestedByteCount;
    d_txPacket[p++] = deviceId;
    d_txPacket[p++] = startAddress;

    uchar dataIndex = deviceId == CM730::ID_CM ? 0 : deviceId;
    data[dataIndex].startAddress = startAddress;
    data[dataIndex].length = requestedByteCount;
  };

  writeDeviceRequest(CM730::ID_CM, cmMin, cmMax);

  for (int id = 1; id <= 20; id++)
    writeDeviceRequest(id, mxMin, mxMax);

  d_txPacket[LENGTH] = p - PARAMETER + 2; // Include one byte each for instruction and checksum
}

//////////
////////// BulkReadTable
//////////

BulkReadTable const& BulkRead::getBulkReadData(uchar id) const
{
  return data[id == CM730::ID_CM ? 0 : id];
}

BulkReadTable::BulkReadTable()
: startAddress(0),
  length(0)
{
  for (int i = 0; i < MX28::MAXNUM_ADDRESS; i++)
    table[i] = 0;
}

uchar BulkReadTable::readByte(uchar address) const
{
  assert(address >= startAddress && address < (startAddress + length));
  return table[address];
}

ushort BulkReadTable::readWord(uchar address) const
{
  assert(address >= startAddress && address < (startAddress + length));
  return CM730::makeWord(table[address], table[address+1]);
}

//////////
////////// CM730
//////////

//// Static members

string CM730::getCommResultName(CommResult responseCode)
{
  switch(responseCode)
  {
    case CommResult::SUCCESS:     return "SUCCESS";
    case CommResult::TX_CORRUPT:  return "TX_CORRUPT";
    case CommResult::TX_FAIL:     return "TX_FAIL";
    case CommResult::RX_FAIL:     return "RX_FAIL";
    case CommResult::RX_TIMEOUT:  return "RX_TIMEOUT";
    case CommResult::RX_CORRUPT:  return "RX_CORRUPT";

    default:          return "UNKNOWN";
  }
}

string CM730::getInstructionName(uchar instructionId)
{
  switch(instructionId)
  {
    case INST_PING:       return "PING";
    case INST_READ:       return "READ";
    case INST_WRITE:      return "WRITE";
    case INST_REG_WRITE:  return "REG_WRITE";
    case INST_ACTION:     return "ACTION";
    case INST_RESET:      return "RESET";
    case INST_SYNC_WRITE: return "SYNC_WRITE";
    case INST_BULK_READ:  return "BULK_READ";

    default:              return "UNKNOWN";
  }
}

//// Instance members

CM730::CM730(unique_ptr<CM730Platform> platform)
: d_platform(move(platform))
{}

CM730::~CM730()
{
  disconnect();
}

CommResult CM730::txRxPacket(uchar *txpacket, uchar *rxpacket, uchar priority, BulkRead* bulkRead = nullptr)
{
  // TODO: assert fails, fix this
  //assert(ThreadUtil::isMotionLoopThread());

  int length = txpacket[LENGTH] + 4;

  txpacket[0] = 0xFF;
  txpacket[1] = 0xFF;
  txpacket[length - 1] = calculateChecksum(txpacket);

  if (DEBUG_PRINT)
  {
    cout << "[CM730::txRxPacket] Transmitting '" << getInstructionName(txpacket[INSTRUCTION]) << "' instruction" << endl;
    cout << "[CM730::txRxPacket]   TX[" << length << "] ";
    cout << hex << setfill('0');
    for (int n = 0; n < length; n++)
      cout << " " << setw(2) << (int)txpacket[n];
    cout << dec << endl;
  }

  CommResult res = CommResult::TX_FAIL;

  if (length < (MAXNUM_TXPARAM + 6))
  {
    // Throw away any unprocessed inbound bytes lingering in the buffer
    d_platform->clearPort();

    // Send the instruction packet
    int bytesWritten = d_platform->writePort(txpacket, length);

    if (bytesWritten == length)
    {
      // Now, handle the response...

      if (txpacket[ID] != ID_BROADCAST)
      {
        int expectedLength = txpacket[INSTRUCTION] == INST_READ
          ? txpacket[PARAMETER+1] + 6
          : 6;

        d_platform->setPacketTimeout(length);

        int receivedCount = 0;
        while (true)
        {
          length = d_platform->readPort(&rxpacket[receivedCount], expectedLength - receivedCount);
          if (length && DEBUG_PRINT)
          {
            cout << "[CM730::txRxPacket]   RX[" << length << "] " << hex << setfill('0');
            for (int n = 0; n < length; n++)
              cout << " " << setw(2) << (int)rxpacket[receivedCount + n];
            cout << dec << endl;
          }
          receivedCount += length;

          if (receivedCount == expectedLength)
          {
            // Find packet header
            // The packet must start with 0xFFFF
            // Walk through until we find it
            int i;
            for (i = 0; i < (receivedCount - 1); i++)
            {
              if (rxpacket[i] == 0xFF && rxpacket[i+1] == 0xFF)
                break;

              if (i == (receivedCount - 2) && rxpacket[receivedCount - 1] == 0xFF)
                break;
            }

            if (i == 0)
            {
              // Check checksum
              uchar checksum = calculateChecksum(rxpacket);

              res = rxpacket[receivedCount-1] == checksum ? CommResult::SUCCESS : CommResult::RX_CORRUPT;
              break;
            }
            else
            {
              // Header was found later in the data
              // Move all data forward in the array
              // This will then loop around until the expected number of bytes are read
              for (int j = 0; j < (receivedCount - i); j++)
                rxpacket[j] = rxpacket[j+i];
              receivedCount -= i;
            }
          }
          else if (d_platform->isPacketTimeout())
          {
            res = receivedCount == 0 ? CommResult::RX_TIMEOUT : CommResult::RX_CORRUPT;
            break;
          }
        }
      }
      else if (txpacket[INSTRUCTION] == INST_BULK_READ)
      {
        assert(bulkRead);

        bulkRead->error = rxpacket[ERRBIT];

        int deviceCount = bulkRead->deviceCount;
        int expectedLength = bulkRead->rxLength;

        d_platform->setPacketTimeout(expectedLength*1.5);

        int receivedCount = 0;

        // Read until we get enough bytes, or there's a timeout
        while (true)
        {
          length = d_platform->readPort(&rxpacket[receivedCount], expectedLength - receivedCount);
          if (length && DEBUG_PRINT)
          {
            cout << "[CM730::txRxPacket]   RX[" << length << "] " << hex << setfill('0');
            for (int n = 0; n < length; n++)
              cout << " " << setw(2) << (int)rxpacket[receivedCount + n];
            cout << dec << endl;
          }
          receivedCount += length;

          if (receivedCount == expectedLength)
          {
            res = CommResult::SUCCESS;
            break;
          }
          else if (d_platform->isPacketTimeout())
          {
            res = receivedCount == 0 ? CommResult::RX_TIMEOUT : CommResult::RX_CORRUPT;
            break;
          }
        }

        // Process data for all devices
        while (true)
        {
          // Search for the header: 0xFFFF
          int i;
          for (i = 0; i < receivedCount - 1; i++)
          {
            if (rxpacket[i] == 0xFF && rxpacket[i + 1] == 0xFF)
              break;
            if (i == (receivedCount - 2) && rxpacket[receivedCount - 1] == 0xFF)
              break;
          }

          if (i == 0)
          {
            // Check checksum
            uchar checksum = calculateChecksum(rxpacket);

            if (rxpacket[LENGTH + rxpacket[LENGTH]] == checksum)
            {
              if (DEBUG_PRINT)
                cout << "[CM730::txRxPacket] Bulk read packet " << (int)rxpacket[ID] << " checksum: " << hex << setfill('0') << setw(2) << (int)checksum << dec << endl;

              // Checksum matches
              for (int j = 0; j < (rxpacket[LENGTH] - 2); j++)
              {
                unsigned dataIndex = rxpacket[ID] == CM730::ID_CM ? 0 : rxpacket[ID];
                unsigned address = bulkRead->data[dataIndex].startAddress + j;
                assert(dataIndex < 21);
                assert(address < MX28::MAXNUM_ADDRESS);
                bulkRead->data[dataIndex].table[address] = rxpacket[PARAMETER + j];
              }

              int curPacketLength = LENGTH + 1 + rxpacket[LENGTH];
              expectedLength = receivedCount - curPacketLength;
              for (int j = 0; j <= expectedLength; j++)
                rxpacket[j] = rxpacket[j + curPacketLength];

              receivedCount = expectedLength;
              deviceCount--;
            }
            else
            {
              // Checksum doesn't match
              res = CommResult::RX_CORRUPT;

              log::warning("CM730::txRxPacket") << "Received checksum didn't match";

              for (int j = 0; j <= receivedCount - 2; j++)
                rxpacket[j] = rxpacket[j + 2];

              expectedLength = receivedCount -= 2;
            }

            // Loop until we've copied from all devices
            if (deviceCount == 0)
              break;

            if (receivedCount <= 6)
            {
              if (deviceCount != 0)
                res = CommResult::RX_CORRUPT;
              break;
            }
          }
          else
          {
            // Move bytes forwards in buffer, so that the header is aligned in byte zero
            for (int j = 0; j < (receivedCount - i); j++)
              rxpacket[j] = rxpacket[j + i];
            receivedCount -= i;
          }
        }
      }
      else
      {
        // Broadcast message, always successful as no response expected (?)
        res = CommResult::SUCCESS;
      }
    }
    else
    {
      log::warning("CM730::txRxPacket") << "Failed to write to port: " << bytesWritten << " of " << length << " written";
      res = CommResult::TX_FAIL;
    }
  }
  else
  {
    res = CommResult::TX_CORRUPT;
  }

  if (DEBUG_PRINT)
    cout << "[CM730::txRxPacket] Round trip in " << setprecision(2) << d_platform->getPacketTime() << "ms  (" << getCommResultName(res) << ")" << endl;

  return res;
}

uchar CM730::calculateChecksum(uchar *packet)
{
  uchar checksum = 0;
  for (int i = 2; i < packet[LENGTH] + 3; i++)
    checksum += packet[i];
  return (~checksum);
}

CommResult CM730::bulkRead(BulkRead* bulkRead)
{
  uchar rxpacket[bulkRead->rxLength];

  bulkRead->error = -1;

  assert(bulkRead->getTxPacket()[LENGTH] != 0);

  return txRxPacket(bulkRead->getTxPacket(), rxpacket, 0, bulkRead);
}

CommResult CM730::syncWrite(uchar fromAddress, uchar bytesPerDevice, uchar deviceCount, uchar *parameters)
{
  unsigned txSize = 8 + (bytesPerDevice * deviceCount);
  if (txSize > 143)
    log::warning("CM730::syncWrite") << "Packet of length " << txSize << " exceeds the Dynamixel's inbound buffer size (" << (int)deviceCount << " devices, " << (int)bytesPerDevice << " bytes per device)";
  uchar txpacket[txSize];
  // Sync write instructions do not receive status packet responses, so no buffer is needed.
  uchar* rxpacket = nullptr;

  txpacket[ID]            = (uchar)ID_BROADCAST;
  txpacket[INSTRUCTION]   = INST_SYNC_WRITE;
  txpacket[PARAMETER]     = fromAddress;
  txpacket[PARAMETER + 1] = (bytesPerDevice - 1);

  int n;
  for (n = 0; n < (deviceCount * bytesPerDevice); n++)
    txpacket[PARAMETER + 2 + n] = parameters[n];

  txpacket[LENGTH] = n + 4;

  return txRxPacket(txpacket, rxpacket, 0);
}

CommResult CM730::reset(uchar id)
{
  uchar txpacket[6];
  uchar rxpacket[6];

  txpacket[ID]            = id;
  txpacket[INSTRUCTION]   = INST_RESET;
  txpacket[LENGTH] = 2;

  return txRxPacket(txpacket, rxpacket, 0);
}

bool CM730::connect()
{
  if (!d_platform->openPort())
  {
    log::error("CM730::connect") << "Failed to open CM730 port (either the CM730 is in use by another program, or you do not have root privileges)";
    return false;
  }

  return powerEnable(true);
}

bool CM730::changeBaud(unsigned baud)
{
  if (!d_platform->setBaud(baud))
  {
    log::error("CM730::changeBaud") << "Failed to change baudrate";
    return false;
  }

  return powerEnable(true);
}

bool CM730::powerEnable(bool enable)
{
  log::info("CM730::powerEnable") << "Turning CM730 power " << (enable ? "on" : "off");

  MX28Alarm alarm;
  if (writeByte(CM730::ID_CM, CM730::P_DXL_POWER, enable ? 1 : 0, &alarm) != CommResult::SUCCESS)
  {
    log::error("CM730::powerEnable") << "Comm error turning CM730 power " << (enable ? "on" : "off");
    return false;
  }

  if (alarm.hasError())
  {
    log::error("CM730::powerEnable") << "Error turning CM730 power " << (enable ? "on" : "off") << ": " << alarm;
    return false;
  }

  // TODO why is this sleep here?
  d_platform->sleep(300); // milliseconds

  return true;
}

bool CM730::torqueEnable(bool enable)
{
  log::info("CM730::torqueEnable") << "" << (enable ? "Enabling" : "Disabling") << " all joint torque";

  bool allSuccessful = true;
  MX28Alarm error;
  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
  {
    auto res = writeByte(jointId, MX28::P_TORQUE_ENABLE, enable ? 1 : 0, &error);

    if (res != CommResult::SUCCESS)
    {
      log::error("CM730::torqueEnable") << "Comm error for " << JointName::getName(jointId) << " (" << (int)jointId << "): " << getCommResultName(res);
      allSuccessful = false;
    }

    if (error.hasError())
    {
      log::error("CM730::torqueEnable") << "Error for " << JointName::getName(jointId) << " (" << (int)jointId << "): " << error;
      allSuccessful = false;
    }
  }
  return allSuccessful;
}

void CM730::disconnect()
{
  if (d_platform->isPortOpen())
  {
    log::verbose("CM730::disconnect") << "Disconnecting from CM730";

    // Set eye/panel LEDs to indicate disconnection
    MX28Alarm alarm1;
    MX28Alarm alarm2;
    MX28Alarm alarm3;
    writeWord(CM730::ID_CM, CM730::P_LED_HEAD_L, CM730::color2Value(0, 255, 0), &alarm1);
    writeWord(CM730::ID_CM, CM730::P_LED_EYE_L,  CM730::color2Value(0,   0, 0), &alarm2);
    writeByte(CM730::ID_CM, CM730::P_LED_PANEL,  0, &alarm3);

    MX28Alarm alarm = alarm1.getFlags() | alarm2.getFlags() | alarm3.getFlags();

    if (alarm.hasError())
    {
      log::error("CM730::disconnect") << "Error setting eye/head/panel LED colours: " << alarm;
      return false;
    }

    powerEnable(false);

    d_platform->closePort();
  }

bool CM730::isPowerEnabled()
{
  uchar value;
  MX28Alarm alarm;

  if (readByte(CM730::ID_CM, CM730::P_DXL_POWER, &value, &alarm) != CommResult::SUCCESS)
  {
    log::error("CM730::isPowerEnabled") << "Comm error reading CM730 power level";
    return false;
  }

  if (alarm.hasError())
  {
    log::error("CM730::isPowerEnabled") << "Error reading CM730 power level: " << alarm;
    return false;
  }

  return value == 1;
}

CommResult CM730::ping(uchar id, MX28Alarm* error)
{
  uchar txpacket[6];
  uchar rxpacket[6];

  txpacket[ID]           = id;
  txpacket[INSTRUCTION]  = INST_PING;
  txpacket[LENGTH]       = 2;

  CommResult result = txRxPacket(txpacket, rxpacket, 2);

  if (result == CommResult::SUCCESS && id != ID_BROADCAST)
  {
    if (error != nullptr)
      *error = rxpacket[ERRBIT];
  }

  return result;
}

CommResult CM730::readByte(uchar id, uchar address, uchar *pValue, MX28Alarm* error)
{
  uchar txpacket[8];
  uchar rxpacket[7];

  txpacket[ID]           = id;
  txpacket[INSTRUCTION]  = INST_READ;
  txpacket[PARAMETER]    = address;
  txpacket[PARAMETER+1]  = 1;
  txpacket[LENGTH]       = 4;

  CommResult result = txRxPacket(txpacket, rxpacket, 2);

  if (result == CommResult::SUCCESS)
  {
    *pValue = rxpacket[PARAMETER];
    if (error != nullptr)
      *error = rxpacket[ERRBIT];
  }

  return result;
}

CommResult CM730::readWord(uchar id, uchar address, ushort *pValue, MX28Alarm* error)
{
  uchar txpacket[8];
  uchar rxpacket[8];

  txpacket[ID]           = id;
  txpacket[INSTRUCTION]  = INST_READ;
  txpacket[PARAMETER]    = address;
  txpacket[PARAMETER+1]  = 2;
  txpacket[LENGTH]       = 4;

  CommResult result = txRxPacket(txpacket, rxpacket, 2);

  if (result == CommResult::SUCCESS)
  {
    *pValue = makeWord(rxpacket[PARAMETER], rxpacket[PARAMETER + 1]);

    if (error != nullptr)
      *error = rxpacket[ERRBIT];
  }

  return result;
}

CommResult CM730::readTable(uchar id, uchar fromAddress, uchar toAddress, uchar *table, MX28Alarm* error)
{
  int length = toAddress - fromAddress + 1;

  uchar txpacket[8];
  uchar rxpacket[6 + length];

  txpacket[ID]           = id;
  txpacket[INSTRUCTION]  = INST_READ;
  txpacket[PARAMETER]    = fromAddress;
  txpacket[PARAMETER+1]  = length;
  txpacket[LENGTH]       = 4;

  CommResult result = txRxPacket(txpacket, rxpacket, 1);

  if (result == CommResult::SUCCESS)
  {
    for (int i = 0; i < length; i++)
      table[fromAddress + i] = rxpacket[PARAMETER + i];

    if (error != nullptr)
      *error = rxpacket[ERRBIT];
  }

  return result;
}

CommResult CM730::writeByte(uchar id, uchar address, uchar value, MX28Alarm* error)
{
  uchar txpacket[8];
  uchar rxpacket[6];

  txpacket[ID]           = id;
  txpacket[INSTRUCTION]  = INST_WRITE;
  txpacket[PARAMETER]    = address;
  txpacket[PARAMETER+1]  = value;
  txpacket[LENGTH]       = 4;

  CommResult result = txRxPacket(txpacket, rxpacket, 2);

  if (result == CommResult::SUCCESS && id != ID_BROADCAST)
  {
    if (error != nullptr)
      *error = rxpacket[ERRBIT];
  }

  return result;
}

CommResult CM730::writeWord(uchar id, uchar address, ushort value, MX28Alarm* error)
{
  uchar txpacket[9];
  uchar rxpacket[6];

  txpacket[ID]           = id;
  txpacket[INSTRUCTION]  = INST_WRITE;
  txpacket[PARAMETER]    = address;
  txpacket[PARAMETER+1]  = (uchar)getLowByte(value);
  txpacket[PARAMETER+2]  = (uchar)getHighByte(value);
  txpacket[LENGTH]       = 5;

  CommResult result = txRxPacket(txpacket, rxpacket, 2);

  if (result == CommResult::SUCCESS && id != ID_BROADCAST)
  {
    if (error != nullptr)
      *error = rxpacket[ERRBIT];
  }

  return result;
}
