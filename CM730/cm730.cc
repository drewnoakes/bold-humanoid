#include "cm730.hh"

#include <cassert>
#include <iostream>
#include <iomanip>

#include "../JointId/jointid.hh"

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

int BulkReadTable::readWord(uchar address) const
{
  assert(address >= startAddress && address < (startAddress + length));
  return CM730::makeWord(table[address], table[address+1]);
}

//////////
////////// CM730
//////////

CM730::CM730(shared_ptr<CM730Platform> platform)
: DEBUG_PRINT(false),
  d_platform(platform)
{}

CM730::~CM730()
{
  disconnect();
}

CommResult CM730::txRxPacket(uchar *txpacket, uchar *rxpacket, uchar priority, shared_ptr<BulkRead> bulkRead = nullptr)
{
  if (priority > 1)
    d_platform->lowPriorityWait();
  if (priority > 0)
    d_platform->midPriorityWait();
  d_platform->highPriorityWait();

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

              cerr << "[CM730::txRxPacket] Received checksum didn't match" << endl;
              
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
      cerr << "[CM730::txRxPacket] Failed to write to port: " << bytesWritten << " of " << length << " written" << endl;
      res = CommResult::TX_FAIL;
    }
  }
  else
  {
    res = CommResult::TX_CORRUPT;
  }

  if (DEBUG_PRINT)
    cout << "[CM730::txRxPacket] Round trip in " << setprecision(2) << d_platform->getPacketTime() << "ms  (" << getCommResultName(res) << ")" << endl;

  d_platform->highPriorityRelease();
  if (priority > 0)
    d_platform->midPriorityRelease();
  if (priority > 1)
    d_platform->lowPriorityRelease();

  return res;
}

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

uchar CM730::calculateChecksum(uchar *packet)
{
  uchar checksum = 0;
  for (int i = 2; i < packet[LENGTH] + 3; i++)
    checksum += packet[i];
  return (~checksum);
}

CommResult CM730::bulkRead(shared_ptr<BulkRead> bulkRead)
{
  uchar rxpacket[bulkRead->rxLength];

  bulkRead->error = -1;

  assert(bulkRead->getTxPacket()[LENGTH] != 0);

  return txRxPacket(bulkRead->getTxPacket(), rxpacket, 0, bulkRead);
}

CommResult CM730::syncWrite(uchar start_addr, uchar each_length, uchar number, uchar *pParam)
{
  unsigned txSize = 8 + (each_length * number);
  if (txSize > 143)
    cerr << "[CM730::SyncWrite] Packet of length " << txSize << " exceeds the Dynamixel's inbound buffer size" << endl;
  uchar txpacket[txSize];
  // Sync write instructions do not receive status packet responses, so no buffer is needed.
  uchar* rxpacket = nullptr;

  txpacket[ID]            = (uchar)ID_BROADCAST;
  txpacket[INSTRUCTION]   = INST_SYNC_WRITE;
  txpacket[PARAMETER]     = start_addr;
  txpacket[PARAMETER + 1] = (each_length - 1);

  int n;
  for (n = 0; n < (number * each_length); n++)
    txpacket[PARAMETER + 2 + n] = pParam[n];

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
    cerr << "[CM730::connect] Failed to open CM730 port" << endl;
    cerr << "[CM730::connect] Either the CM730 is in use by another program, or you do not have root privileges" << endl;
    return false;
  }

  return dxlPowerOn();
}

bool CM730::changeBaud(unsigned baud)
{
  if (d_platform->setBaud(baud) == false)
  {
    cerr << "[CM730::changeBaud] Failed to change baudrate" << endl;
    return false;
  }

  return dxlPowerOn();
}

bool CM730::dxlPowerOn()
{
  cout << "[CM730::dxlPowerOn] Turning Dynamixel power on" << endl;
  
  if (writeByte(CM730::ID_CM, CM730::P_DXL_POWER, 1, 0) == CommResult::SUCCESS)
  {
//        WriteWord(CM730::ID_CM, CM730::P_LED_HEAD_L, MakeColor(255, 128, 0), 0);
    // TODO why is this sleep here?
    d_platform->sleep(300); // about 300msec
  }
  else
  {
    cerr << "[CM730::dxlPowerOn] Failed to change Dynamixel power" << endl;
    return false;
  }

  return true;
}

void CM730::torqueEnable(bool enable)
{
  uchar error;
  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
  {
    writeByte(jointId, MX28::P_TORQUE_ENABLE, enable ? 1 : 0, &error);
    if (error != 0)
    {
      // TODO better reporting of error, across all CM730 operations
      cerr << "[CM730::torqueEnable] error for joint ID " << jointId << ": 0x" << hex << jointId << dec << endl;
    }
  }
}

void CM730::disconnect()
{
  // Make the Head LED to green
  //WriteWord(CM730::ID_CM, CM730::P_LED_HEAD_L, MakeColor(0, 255, 0), 0);
  const uchar txpacket[] = {0xFF, 0xFF, 0xC8, 0x05, 0x03, 0x1A, 0xE0, 0x03, 0x32};
  d_platform->writePort(txpacket, 9);

  d_platform->closePort();
}

CommResult CM730::ping(uchar id, uchar *error)
{
  uchar txpacket[6];
  uchar rxpacket[6];

  txpacket[ID]           = id;
  txpacket[INSTRUCTION]  = INST_PING;
  txpacket[LENGTH]       = 2;

  CommResult result = txRxPacket(txpacket, rxpacket, 2);

  if (result == CommResult::SUCCESS && id != ID_BROADCAST)
  {
    if (error != 0)
      *error = rxpacket[ERRBIT];
  }

  return result;
}

CommResult CM730::readByte(uchar id, uchar address, uchar *pValue, uchar *error)
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
    if (error != 0)
      *error = rxpacket[ERRBIT];
  }

  return result;
}

CommResult CM730::readWord(uchar id, uchar address, int *pValue, uchar *error)
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

    if (error != 0)
      *error = rxpacket[ERRBIT];
  }

  return result;
}

CommResult CM730::readTable(uchar id, uchar start_addr, uchar end_addr, uchar *table, uchar *error)
{
  int length = end_addr - start_addr + 1;

  uchar txpacket[8];
  uchar rxpacket[6 + length];

  txpacket[ID]           = id;
  txpacket[INSTRUCTION]  = INST_READ;
  txpacket[PARAMETER]    = start_addr;
  txpacket[PARAMETER+1]  = length;
  txpacket[LENGTH]       = 4;

  CommResult result = txRxPacket(txpacket, rxpacket, 1);

  if (result == CommResult::SUCCESS)
  {
    for (int i = 0; i < length; i++)
      table[start_addr + i] = rxpacket[PARAMETER + i];

    if (error != 0)
      *error = rxpacket[ERRBIT];
  }

  return result;
}

CommResult CM730::writeByte(uchar id, uchar address, uchar value, uchar *error)
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
    if (error != 0)
      *error = rxpacket[ERRBIT];
  }

  return result;
}

CommResult CM730::writeWord(uchar id, uchar address, int value, uchar *error)
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
    if (error != 0)
      *error = rxpacket[ERRBIT];
  }

  return result;
}
