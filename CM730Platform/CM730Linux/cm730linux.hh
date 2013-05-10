#pragma once

#include <semaphore.h>
#include <string>

#include "../CM730Platform/cm730platform.hh"

namespace bold
{
  class CM730Linux : public CM730Platform
  {
  private:
    /// The FD for the socket connected to the CM730
    int d_socket;

    /// Timestamp of when we start waiting for a response (status) packet, in millis
    double d_packetStartTimeMillis;

    /// Amount of time to wait for a packet, relative to d_PacketStartTime, in millis
    double d_packetWaitTimeMillis;

    /// The estimated amount of time required to send a single byte, in millis. Calculated from baud rate.
    double d_byteTransferTimeMillis;

    /// A string that defines the port of the CM730
    std::string d_portName;

    sem_t d_semIdLow;
    sem_t d_semIdMid;
    sem_t d_semIdHigh;

    unsigned long d_txByteCount;
    unsigned long d_rxByteCount;

  public:
    CM730Linux(std::string name);
    ~CM730Linux();

    void setPortName(std::string name) { d_portName = name; }
    std::string getPortName() const { return d_portName; }

    unsigned long getReceivedByteCount() const override { return d_rxByteCount; }
    unsigned long getTransmittedByteCount() const override { return d_txByteCount; }
    void resetByteCounts() override { d_rxByteCount = d_txByteCount = 0; }

    bool openPort() override;
    bool setBaud(int baud) override;
    void closePort() override;
    void clearPort() override;
    int writePort(unsigned char const* packet, std::size_t numPacket) override;
    int readPort(unsigned char* packet, std::size_t numPacket) override;

    void lowPriorityWait() override;
    void midPriorityWait() override;
    void highPriorityWait() override;
    void lowPriorityRelease() override;
    void midPriorityRelease() override;
    void highPriorityRelease() override;

    void setPacketTimeout(int lenPacket) override;
    bool isPacketTimeout() override;
    double getPacketTime() override;

    virtual void sleep(double msec) override;
  };
}
