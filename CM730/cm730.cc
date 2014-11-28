#include "cm730.hh"

#include <iomanip>
#include <numeric>

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
  d_rxLength = ((uchar)JointId::DEVICE_COUNT + (uchar)1) * 6u;

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
  ASSERT(id != 0);
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
  ASSERT((uchar)address >= (uchar)d_startAddress && (uchar)address < ((uchar)d_startAddress + d_length));

  return d_table.at(address);
}

ushort BulkReadTable::readWord(uchar address) const
{
  ASSERT(address >= d_startAddress && address < (d_startAddress + d_length - 1));

  return CM730::makeWord(d_table[address], d_table[address + 1]);
}

//////////
////////// CM730
//////////

//// Static members

string bold::getCommResultName(CommResult res)
{
  switch (res)
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
    writeWord(CM730Table::LED_HEAD_L, CM730::color2Value(0, 255, 0), &alarm1);
    writeWord(CM730Table::LED_EYE_L,  CM730::color2Value(0,   0, 0), &alarm2);
    writeByte(CM730Table::LED_PANEL,  0, &alarm3);

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
    auto res = writeByte(jointId, (uchar)MX28Table::TORQUE_ENABLE, enable ? 1 : 0, &error);

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
//   if (readByte(CM730::ID_CM, CM730Table::DXL_POWER, &value, &alarm) != CommResult::SUCCESS)
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
  if (writeByte(CM730Table::DXL_POWER, enable ? 1 : 0, &alarm) != CommResult::SUCCESS)
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


CommResult CM730::readByte(CM730Table address, uchar* value, MX28Alarm* error)
{
  return readByte(ID_CM, (uchar)address, value, error);
}

CommResult CM730::readByte(uchar id, MX28Table address, uchar* value, MX28Alarm* error)
{
  return readByte(id, (uchar)address, value, error);
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


CommResult CM730::writeByte(uchar id, MX28Table address, uchar value, MX28Alarm* error)
{
  ASSERT((uchar)address < (uchar)MX28Table::MAXNUM_ADDRESS);
  return writeByte(id, (uchar)address, value, error);
}

CommResult CM730::writeByte(CM730Table address, uchar value, MX28Alarm* error)
{
  ASSERT((uchar)address < (uchar)CM730Table::MAXNUM_ADDRESS);
  return writeByte(ID_CM, (uchar)address, value, error);
}

CommResult CM730::writeByte(uchar id, uchar address, uchar value, MX28Alarm* error)
{
  ASSERT(d_platform->isPortOpen());

  uchar txpacket[8];
  uchar rxpacket[6];

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


CommResult CM730::writeWord(uchar id, MX28Table address, ushort value, MX28Alarm* error)
{
  ASSERT((uchar)address < (uchar)MX28Table::MAXNUM_ADDRESS);
  return writeByte(id, (uchar)address, value, error);
}

CommResult CM730::writeWord(CM730Table address, ushort value, MX28Alarm* error)
{
  ASSERT((uchar)address < (uchar)CM730Table::MAXNUM_ADDRESS);
  return writeByte(ID_CM, (uchar)address, value, error);
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
  ASSERT(fromAddress < (uchar)MX28Table::MAXNUM_ADDRESS);
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

int findPacketHeaderIndex(const uchar* data, uint length)
{
  // The packet must start with 0xFFFF. Walk through until we find it.

  bool hasFlag = false;

  for (uint index = 0; index < length; index++)
  {
    if (data[index] == 0xFF)
    {
      if (hasFlag)
        return index - 1;
      hasFlag = true;
      continue;
    }
    else
    {
      hasFlag = false;
    }
  }

  return -1;
}

CommResult CM730::readPackets(uchar* buffer, const uint bufferLength, std::function<bool(uchar const*)> callback)
{
  uint receivedCount = 0;

  while (true)
  {
    const uint bytesNeeded = bufferLength - receivedCount;

    if (bytesNeeded != 0)
    {
      int bytesRead = d_platform->readPort(&buffer[receivedCount], bytesNeeded);

      if (DEBUG_PRINT && bytesRead > 0)
      {
        cout << "[CM730::readPackets]   RX[" << bytesRead << "]" << hex << setfill('0');
        for (int n = 0; n < bytesRead; n++)
          cout << " " << setw(2) << (int)buffer[receivedCount + n];
        cout << dec << endl;
      }

//      if (bytesRead == 0)
//      {
//        // Sleep for a very short time to avoid keeping the CPU at 100% utilisation
//        // TODO review this sleep duration
//        usleep(15);
//      }

      if (bytesRead < 0)
      {
        log::error("CM730::readPackets") << "Error reading from CM730: " << strerror(errno) << " (" << errno << ")";
        return CommResult::TX_FAIL;
      }
      else
      {
        receivedCount += bytesRead;
      }
    }

    if (receivedCount == (uint)bufferLength)
    {
      // We have the required number of bytes. Now check if it looks valid.

      int i = findPacketHeaderIndex(buffer, bufferLength);

      if (i == -1)
      {
        log::warning("CM730::readPackets") << "No header found in response data";
        return CommResult::RX_CORRUPT;
      }

      if (i != 0)
      {
        // Move bytes forwards in buffer, so that the header is aligned in byte zero.
        // Note that where multiple packets are located consecutively, we only perform this adjustment
        // for the first one.
        std::copy(
          &buffer[i],
          &buffer[bufferLength + 1],
          &buffer[0]);

        // Reduce the count of bytes received, so that we request more bytes when we loop around again
        receivedCount -= i;
        continue;
      }

      // We've found our first packet to start processing the message

      ASSERT(i == 0);

      uchar const* head = buffer;
      uint bytesLeft = bufferLength;

      while (bytesLeft != 0)
      {
        // Validate checksum
        const short calculatedChecksum = calculateChecksum(head, bytesLeft);

        if (findPacketHeaderIndex(head, bufferLength) != 0)
        {
          log::warning("CM730::readPackets") << "Invalid packet header";
          return CommResult::RX_CORRUPT;
        }

        if (calculatedChecksum == - 1)
        {
          log::warning("CM730::readPackets") << "Invalid packet length advertised";
          return CommResult::RX_CORRUPT;
        }

        // Now that the length has been verified, look it up
        const uchar observedChecksum = head[LENGTH + head[LENGTH]];

        if (observedChecksum != calculatedChecksum)
        {
          log::warning("CM730::readPackets") << "Invalid checksum";
          return CommResult::RX_CORRUPT;
        }

        if (DEBUG_PRINT)
          cout << "[CM730::readPackets] Processing packet " << (int)head[ID]
            << " checksum: " << hex << setfill('0') << setw(2) << (int)observedChecksum << dec << endl;

        bool moreToProcess = callback(head);

        if (!moreToProcess)
          return CommResult::SUCCESS;

        uint packetLength = LENGTH + head[LENGTH] + 1; // preamble, data and checksum
        head += packetLength;
        bytesLeft -= packetLength;
      }

      return CommResult::SUCCESS;
    }

    if (d_platform->isPacketTimeout())
    {
      log::error("CM730::readPackets")
        << "Timeout waiting for bulk read response (" << d_platform->getPacketTimeoutMillis()
        << " ms) -- " << receivedCount << " of " << bufferLength << " bytes read";
      return receivedCount == 0 ? CommResult::RX_TIMEOUT : CommResult::RX_CORRUPT;
    }
  }
}

CommResult CM730::txRxPacket(uchar* txpacket, uchar* rxpacket, BulkRead* bulkRead)
{
  ASSERT(ThreadUtil::isMotionLoopThread());
  ASSERT(d_platform->isPortOpen());

  //
  // WRITE DATA
  //

  uint length = txpacket[LENGTH] + 4;
  short checksum = calculateChecksum(txpacket, length);

  ASSERT(checksum >= 0);

  txpacket[0] = 0xFF;
  txpacket[1] = 0xFF;
  txpacket[length - 1] = (uchar)checksum;

  if (DEBUG_PRINT)
  {
    cout << "[CM730::txRxPacket] Transmitting '" << getInstructionName(txpacket[INSTRUCTION]) << "' instruction" << endl;
    cout << "[CM730::txRxPacket]   TX[" << length << "]";
    cout << hex << setfill('0');
    for (uint n = 0; n < length; n++)
      cout << " " << setw(2) << (int)txpacket[n];
    cout << dec << endl;
  }

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

  //
  // READ DATA
  //

  CommResult commResult;

  if (txpacket[ID] != ID_BROADCAST)
  {
    uint rxPacketLength = txpacket[INSTRUCTION] == instruction::Read
      ? txpacket[PARAMETER+1] + 6
      : 6;

    d_platform->setPacketTimeout(length);

    // We will only have one packet in the response, and so long as it has valid structure,
    // the communication was successful. The response doesn't contain anything to process.
    commResult = readPackets(rxpacket, rxPacketLength, [](uchar const* packet) { return false; });
  }
  else if (txpacket[INSTRUCTION] == instruction::BulkRead)
  {
    ASSERT(bulkRead);

    bulkRead->setError(rxpacket[ERRBIT]);

    uchar deviceCount = (uchar)JointId::DEVICE_COUNT + 1;

    d_platform->setPacketTimeout(static_cast<uint>(bulkRead->getRxLength() * 1.5));

    auto devicePacketCallback = [this,&deviceCount,&bulkRead](uchar const* packet) -> bool
    {
      // Copy data from the packet to BulkReadTable
      auto& table = bulkRead->getBulkReadData(packet[ID]);

      // The number of data bytes is equal to the packet's advertised length, minus two (checksum and length bytes)
      const uchar dataByteCount = packet[LENGTH] - (uchar)2;

      ASSERT(table.getStartAddress() + dataByteCount < (uchar)MX28Table::MAXNUM_ADDRESS);

      std::copy(
        &packet[PARAMETER],
        &packet[PARAMETER + dataByteCount + 1],
        table.getData() + table.getStartAddress());

      deviceCount--;
      return deviceCount != 0;
    };

    // We will have one response packet per device, and we'll be called back once per
    commResult = readPackets(rxpacket, bulkRead->getRxLength(), devicePacketCallback);
  }
  else
  {
    // Broadcast message, always successful as no response expected (?)
    commResult = CommResult::SUCCESS;
  }

  if (DEBUG_PRINT)
    cout << "[CM730::txRxPacket] Round trip in " << setprecision(2) << d_platform->getPacketTime()
         << "ms  (" << getCommResultName(commResult) << ")" << endl;

  return commResult;
}

short CM730::calculateChecksum(uchar const *packet, uint bufferLength)
{
  // The packet knows how long it is
  uint packetLength = packet[LENGTH] + 3;

  // Ensure it isn't out of bounds
  if (packetLength > bufferLength)
    return -1;

  // Return the two's complement of all bytes summed, excluding the first two bytes
  return (uchar)~std::accumulate(&packet[2], &packet[packetLength], (uchar)0);
}
