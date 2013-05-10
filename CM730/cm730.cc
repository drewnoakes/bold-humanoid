#include "cm730.hh"

#include <cassert>
#include <iostream>
#include <iomanip>

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

BulkReadData::BulkReadData()
: start_address(0),
  length(0),
  error(-1)
{
  for (int i = 0; i < MX28::MAXNUM_ADDRESS; i++)
    table[i] = 0;
}

int BulkReadData::readByte(int address) const
{
  if (address >= start_address && address < (start_address + length))
    return (int)table[address];

  return 0;
}

int BulkReadData::readWord(int address) const
{
  if (address >= start_address && address < (start_address + length))
    return CM730::makeWord(table[address], table[address+1]);

  return 0;
}



CM730::CM730(shared_ptr<CM730Platform> platform)
{
  d_platform = platform;
  DEBUG_PRINT = false;
  d_bulkReadTxPacket[LENGTH] = 0;

  for (int i = 0; i < ID_BROADCAST; i++)
    d_bulkReadData[i] = BulkReadData();
}

CM730::~CM730()
{
  disconnect();
}

int CM730::txRxPacket(uchar *txpacket, uchar *rxpacket, int priority)
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
    cout << endl << "TX: ";
    for (int n = 0; n < length; n++)
      cout << hex << setfill('0') << setw(2) << txpacket[n];
    cout << "INST: " << getInstructionName(txpacket[INSTRUCTION]) << endl;
  }

  int res = TX_FAIL;

  if (length < (MAXNUM_TXPARAM + 6))
  {
    // Throw away any unprocessed inbound bytes
    d_platform->clearPort();

    // Send the instruction packet
    if (d_platform->writePort(txpacket, length) == length)
    {
      // Now, handle the response...

      if (txpacket[ID] != ID_BROADCAST)
      {
        int expectedLength = txpacket[INSTRUCTION] == INST_READ
          ? txpacket[PARAMETER+1] + 6
          : 6;

        d_platform->setPacketTimeout(length);

        if (DEBUG_PRINT)
            cout << "RX: ";

        int receivedCount = 0;
        while(1)
        {
          length = d_platform->readPort(&rxpacket[receivedCount], expectedLength - receivedCount);
          if (DEBUG_PRINT)
          {
            for (int n = 0; n < length; n++)
              cout << hex << setfill('0') << setw(2) << rxpacket[receivedCount + n] << " ";
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
              if (DEBUG_PRINT)
                cout << "CHK: " << hex << setfill('0') << setw(2) << checksum << endl;

              res = rxpacket[receivedCount-1] == checksum ? SUCCESS : RX_CORRUPT;
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
          else
          {
            // Haven't loaded enough data yet... check the clock
            if (d_platform->isPacketTimeout())
            {
              res = receivedCount == 0 ? RX_TIMEOUT : RX_CORRUPT;
              break;
            }
          }
        }
      }
      else if (txpacket[INSTRUCTION] == INST_BULK_READ)
      {
        // The number of devices (Dynamixels?) expected based upon the request
        int deviceCount = (txpacket[LENGTH]-3) / 3;

        int expectedLength = deviceCount * 6;

        for (int x = 0, p = PARAMETER + 1; x < deviceCount; x++)
        {
          int _len = txpacket[p++];
          int _id = txpacket[p++];
          int _addr = txpacket[p++];

          expectedLength += _len;
          d_bulkReadData[_id].length = _len;
          d_bulkReadData[_id].start_address = _addr;
        }

        d_platform->setPacketTimeout(expectedLength*1.5);

        int receivedCount = 0;
        if (DEBUG_PRINT)
          cout << "RX: ";

        while(1)
        {
          length = d_platform->readPort(&rxpacket[receivedCount], expectedLength - receivedCount);
          if (DEBUG_PRINT)
          {
            for (int n = 0; n < length; n++)
               cout << hex << setfill('0') << setw(2) << rxpacket[receivedCount + n] << " ";
          }
          receivedCount += length;

          if (receivedCount == expectedLength)
          {
            res = SUCCESS;
            break;
          }
          else if (d_platform->isPacketTimeout())
          {
            res = receivedCount == 0 ? RX_TIMEOUT : RX_CORRUPT;
            break;
          }
        }

        for (int x = 0, p = PARAMETER + 2; x < deviceCount; x++, p += 3)
        {
          int _id = txpacket[p];
          d_bulkReadData[_id].error = -1;
        }

        while(1)
        {
          int i;
          for (i = 0; i < receivedCount - 1; i++)
          {
            if (rxpacket[i] == 0xFF && rxpacket[i+1] == 0xFF)
              break;
            else if (i == (receivedCount - 2) && rxpacket[receivedCount - 1] == 0xFF)
              break;
          }

          if (i == 0)
          {
            // Check checksum
            uchar checksum = calculateChecksum(rxpacket);
            if (DEBUG_PRINT)
              cout << "CHK:" << hex << setfill('0') << setw(2) << checksum << endl;

            if (rxpacket[LENGTH+rxpacket[LENGTH]] == checksum)
            {
              for (int j = 0; j < (rxpacket[LENGTH]-2); j++)
                d_bulkReadData[rxpacket[ID]].table[d_bulkReadData[rxpacket[ID]].start_address + j] = rxpacket[PARAMETER + j];

              d_bulkReadData[rxpacket[ID]].error = (int)rxpacket[ERRBIT];

              int cur_packet_length = LENGTH + 1 + rxpacket[LENGTH];
              expectedLength = receivedCount - cur_packet_length;
              for (int j = 0; j <= expectedLength; j++)
                rxpacket[j] = rxpacket[j+cur_packet_length];

              receivedCount = expectedLength;
              deviceCount--;
            }
            else
            {
              res = RX_CORRUPT;

              for (int j = 0; j <= receivedCount - 2; j++)
                rxpacket[j] = rxpacket[j+2];

              expectedLength = receivedCount -= 2;
            }

            if (deviceCount == 0)
              break;
            if (receivedCount <= 6)
            {
              if (deviceCount != 0)
                res = RX_CORRUPT;
              break;
            }
          }
          else
          {
            for (int j = 0; j < (receivedCount - i); j++)
              rxpacket[j] = rxpacket[j+i];
            receivedCount -= i;
          }
        }
      }
      else
      {
        // Broadcast message, always successful as no response expected (?)
        res = SUCCESS;
      }
    }
    else
    {
      res = TX_FAIL;
    }
  }
  else
  {
    res = TX_CORRUPT;
  }

  if (DEBUG_PRINT)
    cout << "Time: " << setprecision(2) << d_platform->getPacketTime() << "ms  RETURN: " << getResponseCodeName(res) << endl;

  d_platform->highPriorityRelease();
  if (priority > 0)
    d_platform->midPriorityRelease();
  if (priority > 1)
    d_platform->lowPriorityRelease();

  return res;
}

string CM730::getResponseCodeName(int responseCode)
{
  switch(responseCode)
  {
    case SUCCESS:     return "SUCCESS";
    case TX_CORRUPT:  return "TX_CORRUPT";
    case TX_FAIL:     return "TX_FAIL";
    case RX_FAIL:     return "RX_FAIL";
    case RX_TIMEOUT:  return "RX_TIMEOUT";
    case RX_CORRUPT:  return "RX_CORRUPT";

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

void CM730::makeBulkReadPacket()
{
  // TODO review length of m_BulkReadTxPacket array -- probably too long

  d_bulkReadTxPacket[ID]          = (uchar)ID_BROADCAST;
  d_bulkReadTxPacket[INSTRUCTION] = INST_BULK_READ;

  uchar p = PARAMETER;

  d_bulkReadTxPacket[p++] = (uchar)0x0;

  auto writeDeviceRequest = [&p,this](uchar deviceId, uchar startAddress, uchar endAddress)
  {
    assert(startAddress < endAddress);
    uchar requestedByteCount = endAddress - startAddress + 1;
    d_bulkReadTxPacket[p++] = requestedByteCount;
    d_bulkReadTxPacket[p++] = deviceId;
    d_bulkReadTxPacket[p++] = startAddress;
  };

//if (Ping(CM730::ID_CM, 0) == SUCCESS)
    writeDeviceRequest(CM730::ID_CM, CM730::P_DXL_POWER, CM730::P_VOLTAGE);

  for (int id = 1; id <= NUMBER_OF_JOINTS; id++)
    writeDeviceRequest(id, MX28::P_PRESENT_POSITION_L, MX28::P_PRESENT_TEMPERATURE);

//if (Ping(FSR::ID_L_FSR, 0) == SUCCESS)
//  writeDeviceRequest(FSR::ID_L_FSR, FSR::P_FSR1_L, FSR::P_FSR_Y);

//if (Ping(FSR::ID_R_FSR, 0) == SUCCESS)
//  writeDeviceRequest(FSR::ID_R_FSR, FSR::P_FSR1_L, FSR::P_FSR_Y);

  d_bulkReadTxPacket[LENGTH] = p - PARAMETER + 2; // Include one byte each for instruction and checksum
}

int CM730::bulkRead()
{
  // TODO review allocated space for this packet
  uchar rxpacket[MAXNUM_RXPARAM + 10] = {0, };

  if (d_bulkReadTxPacket[LENGTH] != 0)
    return txRxPacket(d_bulkReadTxPacket, rxpacket, 0);

  makeBulkReadPacket();
  return TX_FAIL;
}

int CM730::syncWrite(int start_addr, int each_length, int number, int *pParam)
{
  assert(number > 0);
  unsigned txSize = 8 + (each_length * number);
  if (txSize > 143)
    cerr << "[CM730::SyncWrite] Packet of length " << txSize << " exceeds the Dynamixel's inbound buffer size" << endl;
  uchar txpacket[txSize];
  // Sync write instructions do not receive status packet responses, so no buffer is needed.
  uchar* rxpacket = nullptr;

  txpacket[ID]            = (uchar)ID_BROADCAST;
  txpacket[INSTRUCTION]   = INST_SYNC_WRITE;
  txpacket[PARAMETER]     = (uchar)start_addr;
  txpacket[PARAMETER + 1] = (uchar)(each_length - 1);

  int n;
  for (n = 0; n < (number * each_length); n++)
    txpacket[PARAMETER + 2 + n] = (uchar)pParam[n];

  txpacket[LENGTH] = n + 4;

  return txRxPacket(txpacket, rxpacket, 0);
}

int CM730::reset(uchar id)
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
  if (d_platform->openPort() == false)
  {
    cerr << "[CM730::connect] Failed to open CM730 port" << endl;
    cerr << "[CM730::connect] Either the CM730 is in use by another program, or you do not have root privileges" << endl;
    return false;
  }

  return dxlPowerOn();
}

bool CM730::changeBaud(int baud)
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
  if (writeByte(CM730::ID_CM, CM730::P_DXL_POWER, 1, 0) == CM730::SUCCESS)
  {
    if (DEBUG_PRINT)
      cout << "[CM730::dxlPowerOn] Succeed to change Dynamixel power" << endl;

//        WriteWord(CM730::ID_CM, CM730::P_LED_HEAD_L, MakeColor(255, 128, 0), 0);
    // TODO why is this sleep here?
    d_platform->sleep(300); // about 300msec
  }
  else
  {
    if (DEBUG_PRINT)
      cerr << "[CM730::dxlPowerOn] Failed to change Dynamixel power" << endl;
    return false;
  }

  return true;
}

void CM730::disconnect()
{
  // Make the Head LED to green
  //WriteWord(CM730::ID_CM, CM730::P_LED_HEAD_L, MakeColor(0, 255, 0), 0);
  const uchar txpacket[] = {0xFF, 0xFF, 0xC8, 0x05, 0x03, 0x1A, 0xE0, 0x03, 0x32};
  d_platform->writePort(txpacket, 9);

  d_platform->closePort();
}

int CM730::ping(int id, uchar *error)
{
  uchar txpacket[6];
  uchar rxpacket[6];

  txpacket[ID]           = (uchar)id;
  txpacket[INSTRUCTION]  = INST_PING;
  txpacket[LENGTH]       = 2;

  int result = txRxPacket(txpacket, rxpacket, 2);

  if (result == SUCCESS && txpacket[ID] != ID_BROADCAST)
  {
    if (error != 0)
      *error = rxpacket[ERRBIT];
  }

  return result;
}

int CM730::readByte(int id, int address, uchar *pValue, uchar *error)
{
  uchar txpacket[8];
  uchar rxpacket[7];

  txpacket[ID]           = (uchar)id;
  txpacket[INSTRUCTION]  = INST_READ;
  txpacket[PARAMETER]    = (uchar)address;
  txpacket[PARAMETER+1]  = 1;
  txpacket[LENGTH]       = 4;

  int result = txRxPacket(txpacket, rxpacket, 2);

  if (result == SUCCESS)
  {
    *pValue = rxpacket[PARAMETER];
    if (error != 0)
      *error = rxpacket[ERRBIT];
  }

  return result;
}

int CM730::readWord(int id, int address, int *pValue, uchar *error)
{
  uchar txpacket[8];
  uchar rxpacket[8];

  txpacket[ID]           = (uchar)id;
  txpacket[INSTRUCTION]  = INST_READ;
  txpacket[PARAMETER]    = (uchar)address;
  txpacket[PARAMETER+1]  = 2;
  txpacket[LENGTH]       = 4;

  int result = txRxPacket(txpacket, rxpacket, 2);

  if (result == SUCCESS)
  {
    *pValue = makeWord(rxpacket[PARAMETER], rxpacket[PARAMETER + 1]);

    if (error != 0)
      *error = rxpacket[ERRBIT];
  }

  return result;
}

int CM730::readTable(int id, int start_addr, int end_addr, uchar *table, uchar *error)
{
  int length = end_addr - start_addr + 1;

  uchar txpacket[8];
  uchar rxpacket[6 + length];

  txpacket[ID]           = (uchar)id;
  txpacket[INSTRUCTION]  = INST_READ;
  txpacket[PARAMETER]    = (uchar)start_addr;
  txpacket[PARAMETER+1]  = (uchar)length;
  txpacket[LENGTH]       = 4;

  int result = txRxPacket(txpacket, rxpacket, 1);

  if (result == SUCCESS)
  {
    for (int i=0; i<length; i++)
      table[start_addr + i] = rxpacket[PARAMETER + i];

    if (error != 0)
      *error = rxpacket[ERRBIT];
  }

  return result;
}

int CM730::writeByte(int id, int address, int value, uchar *error)
{
  uchar txpacket[8];
  uchar rxpacket[6];

  txpacket[ID]           = (uchar)id;
  txpacket[INSTRUCTION]  = INST_WRITE;
  txpacket[PARAMETER]    = (uchar)address;
  txpacket[PARAMETER+1]  = (uchar)value;
  txpacket[LENGTH]       = 4;

  int result = txRxPacket(txpacket, rxpacket, 2);

  if (result == SUCCESS && id != ID_BROADCAST)
  {
    if (error != 0)
      *error = rxpacket[ERRBIT];
  }

  return result;
}

int CM730::writeWord(int id, int address, int value, uchar *error)
{
  uchar txpacket[9];
  uchar rxpacket[6];

  txpacket[ID]           = (uchar)id;
  txpacket[INSTRUCTION]  = INST_WRITE;
  txpacket[PARAMETER]    = (uchar)address;
  txpacket[PARAMETER+1]  = (uchar)getLowByte(value);
  txpacket[PARAMETER+2]  = (uchar)getHighByte(value);
  txpacket[LENGTH]       = 5;

  int result = txRxPacket(txpacket, rxpacket, 2);

  if (result == SUCCESS && id != ID_BROADCAST)
  {
    if (error != 0)
      *error = rxpacket[ERRBIT];
  }

  return result;
}
