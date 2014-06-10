#include "cm730.hh"

#include <iomanip>

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
: d_error((uchar)-1)
{
  // Build a place for the data we read back
  d_data.fill(BulkReadTable());

  // We will receive 6 bytes per device plus an amount varying with the requested address range (added in below)
  d_rxLength = (uchar)JointId::DEVICE_COUNT * 6u;

  // Create a cached TX packet as it'll be identical each time
  d_txPacket[ID]          = CM730::ID_BROADCAST;
  d_txPacket[INSTRUCTION] = instruction::BulkRead;

  uchar p = PARAMETER;

  d_txPacket[p++] = (uchar)0x0;

  auto writeDeviceRequest = [&p,this](uchar deviceId, uchar startAddress, uchar endAddress)
  {
    ASSERT(startAddress < endAddress);

    uchar requestedByteCount = endAddress - startAddress + (uchar)1;

    d_rxLength += requestedByteCount;

    d_txPacket[p++] = requestedByteCount;
    d_txPacket[p++] = deviceId;
    d_txPacket[p++] = startAddress;

    auto& table = d_data.at(deviceId == CM730::ID_CM ? 0 : deviceId);
    table.setStartAddress(startAddress);
    table.setLength(requestedByteCount);
  };

  writeDeviceRequest(CM730::ID_CM, cmMin, cmMax);

  static_assert((uchar)JointId::MIN == 1, "Lowest JointId must be 1 for the data packing scheme to work. CM730 is at index 0.");

  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    writeDeviceRequest(jointId, mxMin, mxMax);

  d_txPacket[LENGTH] = p - (uchar)PARAMETER + (uchar)2; // Include one byte each for instruction and checksum
}

//////////
////////// BulkRead
//////////

BulkReadTable& BulkRead::getBulkReadData(uchar id)
{
  ASSERT(id == CM730::ID_CM || (id >= (uchar)JointId::MIN && id <= (uchar)JointId::MAX));

  return d_data.at(id == CM730::ID_CM ? 0 : id);
}

//////////
////////// BulkReadTable
//////////

BulkReadTable::BulkReadTable()
: d_startAddress(0),
  d_length(0)
{
  d_table.fill(0);
}

uchar BulkReadTable::readByte(uchar address) const
{
  ASSERT(address >= d_startAddress && address < (d_startAddress + d_length));

  return d_table.at(address);
}

ushort BulkReadTable::readWord(uchar address) const
{
  ASSERT(address >= d_startAddress && address < (d_startAddress + d_length - 1));

  return CM730::makeWord(d_table.at(address), d_table.at(address + 1));
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
    default:                      return "UNKNOWN";
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
    default:                     return "UNKNOWN";
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
  ASSERT(d_platform->isPortOpen());

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
  ASSERT(d_platform->isPortOpen());

  uchar txpacket[6];
  uchar rxpacket[6];

  txpacket[ID]            = id;
  txpacket[INSTRUCTION]   = instruction::Reset;
  txpacket[LENGTH] = 2;

  return txRxPacket(txpacket, rxpacket);
}

CommResult CM730::readByte(uchar id, uchar address, uchar *pValue, MX28Alarm* error)
{
  ASSERT(d_platform->isPortOpen());

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
  ASSERT(d_platform->isPortOpen());

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
  ASSERT(d_platform->isPortOpen());

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
  ASSERT(d_platform->isPortOpen());

  uchar rxpacket[bulkRead->getRxLength()];

  bulkRead->clearError();

  ASSERT(bulkRead->getTxPacket()[LENGTH] != 0);

  return txRxPacket(bulkRead->getTxPacket(), rxpacket, bulkRead);
}

CommResult CM730::writeByte(uchar id, uchar address, uchar value, MX28Alarm* error)
{
  ASSERT(d_platform->isPortOpen());

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
  ASSERT(d_platform->isPortOpen());

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
  ASSERT(d_platform->isPortOpen());
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

uint findPacketHeaderIndex(const uchar* data, uint length)
{
  // The packet must start with 0xFFFF. Walk through until we find it.
  uint i;
  for (i = 0; i < (length - 1); i++)
  {
    if (data[i] == 0xFF && data[i+1] == 0xFF)
      break;

    if (i == (length - 2) && data[length - 1] == 0xFF)
      break;
  }
  return i;
}

CommResult CM730::txRxPacket(uchar* txpacket, uchar* rxpacket, BulkRead* bulkRead)
{
  ASSERT(ThreadUtil::isMotionLoopThread());
  ASSERT(d_platform->isPortOpen());

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
        // Find packet header index
        uint i = findPacketHeaderIndex(rxpacket, receivedCount);

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

    bulkRead->setError(rxpacket[ERRBIT]);

    uchar deviceCount = (uchar)JointId::DEVICE_COUNT;

    uint remainingByteCount = bulkRead->getRxLength();

    d_platform->setPacketTimeout(static_cast<uint>(remainingByteCount * 1.5));

    //
    // Read until we get enough bytes, or there's a timeout
    //

    uint receivedCount = 0;

    while (true)
    {
      int bytesRead = d_platform->readPort(&rxpacket[receivedCount], remainingByteCount - receivedCount);

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

      if (receivedCount == remainingByteCount)
      {
        res = CommResult::SUCCESS;
        break;
      }
      else if (d_platform->isPacketTimeout())
      {
        log::error("CM730::txRxPacket") << "Timeout waiting for bulk read response (" << d_platform->getPacketTimeoutMillis() << " ms) -- " << receivedCount << " of " << remainingByteCount << " bytes read";
        return receivedCount == 0 ? CommResult::RX_TIMEOUT : CommResult::RX_CORRUPT;
      }
    }

    //
    // Process response message
    //

    auto removePacketBytes = [&](uint removeCount)
    {
      // TODO this seems a very wasteful approach -- should just operate on memory in a forward-only fashion

      std::copy(
        &rxpacket[removeCount],
        &rxpacket[remainingByteCount + 1],
        &rxpacket[0]);

      remainingByteCount -= removeCount;
    };

    while (true)
    {
      uint i = findPacketHeaderIndex(rxpacket, remainingByteCount);

      if (i == 0)
      {
        // Validate checksum
        const uchar observedChecksum = rxpacket[LENGTH + rxpacket[LENGTH]];
        const uchar expectedChecksum = calculateChecksum(rxpacket);

        if (observedChecksum == expectedChecksum)
        {
          // Checksum matches

          if (DEBUG_PRINT)
            cout << "[CM730::txRxPacket] Bulk read packet " << (int)rxpacket[ID]
                 << " checksum: " << hex << setfill('0') << setw(2) << (int)expectedChecksum << dec << endl;

          // Copy data from rxpacket to BulkReadTable
          auto& table = bulkRead->getBulkReadData(rxpacket[ID]);

          // The number of data bytes is equal to the packet's advertised length, minus two (checksum and length bytes)
          const uchar dataByteCount = rxpacket[LENGTH] - (uchar)2;
          ASSERT(table.getStartAddress() + dataByteCount < MX28::MAXNUM_ADDRESS);
          std::copy(
            &rxpacket[PARAMETER],
            &rxpacket[PARAMETER + dataByteCount + 1],
            table.getData() + table.getStartAddress());

          // Move data forward in the packet
          const uint curPacketLength = LENGTH + 1 + rxpacket[LENGTH];

          removePacketBytes(curPacketLength);

          deviceCount--;
        }
        else
        {
          // Checksum doesn't match
          res = CommResult::RX_CORRUPT;

          log::warning("CM730::txRxPacket") << "Received checksum didn't match";

          removePacketBytes(2);
        }

        // Loop until we've copied from all devices
        if (deviceCount == 0)
          break;

        if (remainingByteCount <= 6)
        {
          if (deviceCount != 0)
            res = CommResult::RX_CORRUPT;
          break;
        }
      }
      else
      {
        // Move bytes forwards in buffer, so that the header is aligned in byte zero
        removePacketBytes(i);
      }
    }
  }
  else
  {
    // Broadcast message, always successful as no response expected (?)
    res = CommResult::SUCCESS;
  }

  if (DEBUG_PRINT)
    cout << "[CM730::txRxPacket] Round trip in " << setprecision(2) << d_platform->getPacketTime()
         << "ms  (" << getCommResultName(res) << ")" << endl;

  return res;
}

uchar CM730::calculateChecksum(uchar *packet)
{
  return ~std::accumulate(&packet[2], &packet[packet[LENGTH] + 3], (uchar)0);
}
