#pragma once

#include <string>

#include "../../CM730Platform/cm730platform.hh"

namespace bold
{
  typedef unsigned char uchar;

  class CM730Linux : public CM730Platform
  {
  public:
    CM730Linux(std::string name);
    ~CM730Linux();

    void setPortName(std::string name) { d_portName = name; }
    std::string getPortName() const { return d_portName; }

    unsigned long getReceivedByteCount() const override { return d_rxByteCount; }
    unsigned long getTransmittedByteCount() const override { return d_txByteCount; }
    void resetByteCounts() override { d_rxByteCount = d_txByteCount = 0; }

    bool openPort() override;
    bool setBaud(unsigned baud) override;
    bool closePort() override;
    bool clearPort() override;
    bool isPortOpen() const override;
    int writePort(uchar const* packet, std::size_t numPacket) override;
    int readPort(uchar* packet, std::size_t numPacket) override;

    void setPacketTimeout(uint lenPacket) override;
    bool isPacketTimeout() override;
    double getPacketTime() override;
    double getPacketTimeoutMillis() const override;

    void sleep(double msec) override;

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

    unsigned long d_txByteCount;
    unsigned long d_rxByteCount;

    //bool d_isPortOpen;

  };
}
