#include "cm730.hh"

#include <iomanip>

#include "../JointId/jointid.hh"
#include "../ThreadUtil/threadutil.hh"
#include "../util/assert.hh"
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
  d_txPacket[ID]          = CM730::ID_BROADCAST;
  d_txPacket[INSTRUCTION] = instruction::BulkRead;

  uchar p = PARAMETER;

  d_txPacket[p++] = (uchar)0x0;

  rxLength = deviceCount * 6;

  auto writeDeviceRequest = [&p,this](uchar deviceId, uchar startAddress, uchar endAddress)
  {
    ASSERT(startAddress < endAddress);

    uchar requestedByteCount = endAddress - startAddress + (uchar)1;

    rxLength += requestedByteCount;

    d_txPacket[p++] = requestedByteCount;
    d_txPacket[p++] = deviceId;
    d_txPacket[p++] = startAddress;

    uchar dataIndex = deviceId == CM730::ID_CM ? 0 : deviceId;
    data[dataIndex].startAddress = startAddress;
    data[dataIndex].length = requestedByteCount;
  };

  writeDeviceRequest(CM730::ID_CM, cmMin, cmMax);

  for (uchar id = 1; id <= 20; id++)
    writeDeviceRequest(id, mxMin, mxMax);

  d_txPacket[LENGTH] = p - (uchar)PARAMETER + (uchar)2; // Include one byte each for instruction and checksum
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
  memset(table, 0, MX28::MAXNUM_ADDRESS);
}

uchar BulkReadTable::readByte(uchar address) const
{
  ASSERT(address >= startAddress && address < (startAddress + length));
  return table[address];
}

ushort BulkReadTable::readWord(uchar address) const
{
  ASSERT(address >= startAddress && address < (startAddress + length));
  return CM730::makeWord(table[address], table[address+1]);
}

//////////
////////// CM730
//////////

//// Static members

string CM730::getCommResultName(CommResult responseCode)
{
  switch (responseCode)
  {
    case CommResult::SUCCESS:     return "SUCCESS";
    case CommResult::TX_CORRUPT:  return "TX_CORRUPT";
    case CommResult::TX_FAIL:     return "TX_FAIL";
    case CommResult::RX_TIMEOUT:  return "RX_TIMEOUT";
    case CommResult::RX_CORRUPT:  return "RX_CORRUPT";

    default:          return "UNKNOWN";
  }
}

string getInstructionName(uchar instructionId)
{
  switch (instructionId)
  {
    case instruction::Ping:      return "PING";
    case instruction::Read:      return "READ";
    case instruction::Write:     return "WRITE";
    case instruction::RegWrite:  return "REG_WRITE";
    case instruction::Action:    return "ACTION";
    case instruction::Reset:     return "RESET";
    case instruction::SyncWrite: return "SYNC_WRITE";
    case instruction::BulkRead:  return "BULK_READ";
    default:                       return "UNKNOWN";
  }
}

//// Instance members

CM730::CM730(unique_ptr<CM730Platform> platform)
: d_platform(move(platform)),
  d_isPowerEnableRequested(false)
{}

CM730::~CM730()
{
  disconnect();
}

bool CM730::connect()
{
  return d_platform->openPort() && powerEnable(true);
}

bool CM730::disconnect()
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

    if (!powerEnable(false))
    {
      log::error("CM730::disconnect") << "Error turning power off";
      return false;
    }

    if (!d_platform->closePort())
    {
      log::error("CM730::disconnect") << "Error closing port";
      return false;
    }
  }

  return true;
}

bool CM730::changeBaud(unsigned baud)
{
  if (!d_platform->setBaud(baud))
  {
    log::error("CM730::changeBaud") << "Failed to change baudrate to " << baud;
    return false;
  }

  return powerEnable(true);
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
      log::error("CM730::torqueEnable") << "Comm error for " << JointName::getEnumName(jointId) << " (" << (int)jointId << "): " << getCommResultName(res);
      allSuccessful = false;
    }

    if (error.hasError())
    {
      log::error("CM730::torqueEnable") << "Error for " << JointName::getEnumName(jointId) << " (" << (int)jointId << "): " << error;
      allSuccessful = false;
    }
  }
  return allSuccessful;
}

bool CM730::isPowerEnabled()
{
  return d_isPowerEnableRequested;
//   uchar value;
//   MX28Alarm alarm;
//
//   static bool commError = false;
//
//   if (readByte(CM730::ID_CM, CM730::P_DXL_POWER, &value, &alarm) != CommResult::SUCCESS)
//   {
//     if (!commError)
//     {
//       log::error("CM730::isPowerEnabled") << "Comm error reading CM730 power level";
//       commError = true;
//     }
//     return false;
//   }
//   else
//   {
//     commError = false;
//   }
//
//   if (alarm.hasError())
//   {
//     log::error("CM730::isPowerEnabled") << "Error reading CM730 power level: " << alarm;
//     return false;
//   }
//
//   return value == 1;
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

  d_isPowerEnableRequested = enable;

  // TODO why is this sleep here?
  d_platform->sleep(300); // milliseconds

  return true;
}

CommResult CM730::ping(uchar id, MX28Alarm* error)
{
  uchar txpacket[6];
  uchar rxpacket[6];

  txpacket[ID]           = id;
  txpacket[INSTRUCTION]  = instruction::Ping;
  txpacket[LENGTH]       = 2;

  CommResult result = txRxPacket(txpacket, rxpacket);

  if (result == CommResult::SUCCESS && id != ID_BROADCAST)
  {
    if (error != nullptr)
      *error = rxpacket[ERRBIT];
  }

  return result;
}

CommResult CM730::reset(uchar id)
{
  uchar txpacket[6];
  uchar rxpacket[6];

  txpacket[ID]            = id;
  txpacket[INSTRUCTION]   = instruction::Reset;
  txpacket[LENGTH] = 2;

  return txRxPacket(txpacket, rxpacket);
}

CommResult CM730::readByte(uchar id, uchar address, uchar *pValue, MX28Alarm* error)
{
  uchar txpacket[8];
  uchar rxpacket[7];

  txpacket[ID]           = id;
  txpacket[INSTRUCTION]  = instruction::Read;
  txpacket[PARAMETER]    = address;
  txpacket[PARAMETER+1]  = 1;
  txpacket[LENGTH]       = 4;

  CommResult result = txRxPacket(txpacket, rxpacket);

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
  txpacket[INSTRUCTION]  = instruction::Read;
  txpacket[PARAMETER]    = address;
  txpacket[PARAMETER+1]  = 2;
  txpacket[LENGTH]       = 4;

  CommResult result = txRxPacket(txpacket, rxpacket);

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
  txpacket[INSTRUCTION]  = instruction::Read;
  txpacket[PARAMETER]    = fromAddress;
  txpacket[PARAMETER+1]  = length;
  txpacket[LENGTH]       = 4;

  CommResult result = txRxPacket(txpacket, rxpacket);

  if (result == CommResult::SUCCESS)
  {
    for (int i = 0; i < length; i++)
      table[fromAddress + i] = rxpacket[PARAMETER + i];

    if (error != nullptr)
      *error = rxpacket[ERRBIT];
  }

  return result;
}

CommResult CM730::bulkRead(BulkRead* bulkRead)
{
  uchar rxpacket[bulkRead->rxLength];

  bulkRead->error = -1;

  ASSERT(bulkRead->getTxPacket()[LENGTH] != 0);

  return txRxPacket(bulkRead->getTxPacket(), rxpacket, bulkRead);
}

CommResult CM730::writeByte(uchar id, uchar address, uchar value, MX28Alarm* error)
{
  uchar txpacket[8];
  uchar rxpacket[6];

  ASSERT( (id == ID_CM && address < CM730::MAXNUM_ADDRESS) || (address < MX28::MAXNUM_ADDRESS) );

  txpacket[ID]           = id;
  txpacket[INSTRUCTION]  = instruction::Write;
  txpacket[PARAMETER]    = address;
  txpacket[PARAMETER+1]  = value;
  txpacket[LENGTH]       = 4;

  CommResult result = txRxPacket(txpacket, rxpacket);

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
  txpacket[LENGTH]       = 5;
  txpacket[INSTRUCTION]  = instruction::Write;
  txpacket[PARAMETER]    = address;
  txpacket[PARAMETER+1]  = (uchar)getLowByte(value);
  txpacket[PARAMETER+2]  = (uchar)getHighByte(value);

  CommResult result = txRxPacket(txpacket, rxpacket);

  if (result == CommResult::SUCCESS && id != ID_BROADCAST)
  {
    if (error != nullptr)
      *error = rxpacket[ERRBIT];
  }

  return result;
}

CommResult CM730::syncWrite(uchar fromAddress, uchar bytesPerDevice, uchar deviceCount, uchar *parameters)
{
  ASSERT(bytesPerDevice != 0);
  ASSERT(bytesPerDevice - 1 <= 255);
  ASSERT(deviceCount != 0);
  ASSERT(fromAddress < MX28::MAXNUM_ADDRESS);
  unsigned paramLength = bytesPerDevice * deviceCount;
  ASSERT(paramLength + 4 <= 255);
  unsigned txSize = 8 + paramLength;
  if (txSize > 143)
    log::warning("CM730::syncWrite") << "Packet of length " << txSize << " exceeds the Dynamixel's inbound buffer size (" << (int)deviceCount << " devices, " << (int)bytesPerDevice << " bytes per device)";
  uchar txpacket[txSize];
  // Sync write instructions do not receive status packet responses, so no buffer is needed.
  uchar* rxpacket = nullptr;

  txpacket[ID]            = ID_BROADCAST;
  txpacket[LENGTH]        = paramLength + 4;
  txpacket[INSTRUCTION]   = instruction::SyncWrite;
  txpacket[PARAMETER]     = fromAddress;
  txpacket[PARAMETER + 1] = bytesPerDevice - 1;

  std::copy(&parameters[0], &parameters[paramLength], &txpacket[PARAMETER + 2]);

  return txRxPacket(txpacket, rxpacket);
}

CommResult CM730::txRxPacket(uchar* txpacket, uchar* rxpacket, BulkRead* bulkRead)
{
  ASSERT(ThreadUtil::isMotionLoopThread());

  uint length = txpacket[LENGTH] + 4;

  txpacket[0] = 0xFF;
  txpacket[1] = 0xFF;
  txpacket[length - 1] = calculateChecksum(txpacket);

  if (DEBUG_PRINT)
  {
    cout << "[CM730::txRxPacket] Transmitting '" << getInstructionName(txpacket[INSTRUCTION]) << "' instruction" << endl;
    cout << "[CM730::txRxPacket]   TX[" << length << "]";
    cout << hex << setfill('0');
    for (uint n = 0; n < length; n++)
      cout << " " << setw(2) << (int)txpacket[n];
    cout << dec << endl;
  }

  CommResult res = CommResult::TX_FAIL;

  if (length >= (MAXNUM_TXPARAM + 6))
  {
    log::error("CM730::txRxPacket") << "Attempt to write " << length << " bytes, which is more than MAXNUM_TXPARAM+6";
    return CommResult::TX_CORRUPT;
  }

  // Throw away any unprocessed inbound bytes lingering in the buffer
  d_platform->clearPort();

  // Send the instruction packet
  int bytesWritten = d_platform->writePort(txpacket, length);

  if (bytesWritten < 0)
  {
    log::error("CM730::txRxPacket") << "Error writing to CM730 port: " << strerror(errno) << " (" << errno << ")";
    return CommResult::TX_FAIL;
  }
  else if ((uint)bytesWritten != length)
  {
    log::warning("CM730::txRxPacket") << "Failed to write entire packet to port: " << bytesWritten << " of " << length << " written";
    return CommResult::TX_FAIL;
  }

  // Now, handle the response...

  if (txpacket[ID] != ID_BROADCAST)
  {
    int expectedLength = txpacket[INSTRUCTION] == instruction::Read
      ? txpacket[PARAMETER+1] + 6
      : 6;

    d_platform->setPacketTimeout(length);

    int receivedCount = 0;
    while (true)
    {
      int bytesRead = d_platform->readPort(&rxpacket[receivedCount], expectedLength - receivedCount);

      if (DEBUG_PRINT && bytesRead > 0)
      {
        cout << "[CM730::txRxPacket]   RX[" << bytesRead << "]" << hex << setfill('0');
        for (int n = 0; n < bytesRead; n++)
          cout << " " << setw(2) << (int)rxpacket[receivedCount + n];
        cout << dec << endl;
      }

      if (bytesRead < 0)
      {
        log::error("CM730::txRxPacket") << "Error reading from CM730 port: " << strerror(errno) << " (" << errno << ")";
        return CommResult::TX_FAIL;
      }
      else
      {
        receivedCount += bytesRead;
      }

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
          log::warning("CM730::txRxPacket") << "Skipping " << i << " byte" << (i==1?"":"s") << " until response header -- possible corruption";
          // Move all data forward in the array
          // This will then loop around until the expected number of bytes are read
          for (int j = 0; j < (receivedCount - i); j++)
            rxpacket[j] = rxpacket[j+i];
          receivedCount -= i;
        }
      }
      else if (d_platform->isPacketTimeout())
      {
        log::error("CM730::txRxPacket") << "Timeout waiting for response (" << d_platform->getPacketTimeoutMillis() << " ms) -- " << receivedCount << " of " << expectedLength << " bytes read";
        return receivedCount == 0 ? CommResult::RX_TIMEOUT : CommResult::RX_CORRUPT;
      }
    }
  }
  else if (txpacket[INSTRUCTION] == instruction::BulkRead)
  {
    ASSERT(bulkRead);

    bulkRead->error = rxpacket[ERRBIT];

    int deviceCount = bulkRead->deviceCount;
    int expectedLength = bulkRead->rxLength;

    d_platform->setPacketTimeout(static_cast<uint>(expectedLength * 1.5));

    uint receivedCount = 0;

    // Read until we get enough bytes, or there's a timeout
    while (true)
    {
      int bytesRead = d_platform->readPort(&rxpacket[receivedCount], expectedLength - receivedCount);

      if (DEBUG_PRINT && bytesRead > 0)
      {
        cout << "[CM730::txRxPacket]   RX[" << bytesRead << "]" << hex << setfill('0');
        for (int n = 0; n < bytesRead; n++)
          cout << " " << setw(2) << (int)rxpacket[receivedCount + n];
        cout << dec << endl;
      }

      if (bytesRead < 0)
      {
        log::error("CM730::txRxPacket") << "Error reading from CM730 port: " << strerror(errno) << " (" << errno << ")";
        return CommResult::TX_FAIL;
      }
      else
      {
        receivedCount += bytesRead;
      }

      if (receivedCount == expectedLength)
      {
        res = CommResult::SUCCESS;
        break;
      }
      else if (d_platform->isPacketTimeout())
      {
        log::error("CM730::txRxPacket") << "Timeout waiting for bulk read response (" << d_platform->getPacketTimeoutMillis() << " ms) -- " << receivedCount << " of " << expectedLength << " bytes read";
        return receivedCount == 0 ? CommResult::RX_TIMEOUT : CommResult::RX_CORRUPT;
      }
    }

    // Process data for all devices
    while (true)
    {
      // Search for the header: 0xFFFF
      uint i;
      for (i = 0; i < receivedCount - 1u; i++)
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
            ASSERT(dataIndex < 21);
            ASSERT(address < MX28::MAXNUM_ADDRESS);
            bulkRead->data[dataIndex].table[address] = rxpacket[PARAMETER + j];
          }

          uint curPacketLength = LENGTH + 1 + rxpacket[LENGTH];
          expectedLength = receivedCount - curPacketLength;
          for (uint j = 0; j <= expectedLength; j++)
            rxpacket[j] = rxpacket[j + curPacketLength];

          receivedCount = expectedLength;
          deviceCount--;
        }
        else
        {
          // Checksum doesn't match
          res = CommResult::RX_CORRUPT;

          log::warning("CM730::txRxPacket") << "Received checksum didn't match";

          for (int j = 0; j <= receivedCount - 2u; j++)
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
        for (uint j = 0; j < (receivedCount - i); j++)
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
