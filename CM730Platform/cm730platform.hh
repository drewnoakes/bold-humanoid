#pragma once

#define MAXNUM_TXPARAM (256)
#define MAXNUM_RXPARAM (1024)

namespace bold
{
  typedef unsigned char uchar;

  /** An abstract base class for classes that provide direct access to the CM730
   * hardware in a means appropriate to the operating system, such as LinuxCM730.
   */
  class CM730Platform
  {
  public:

    /// Opens CM730 port, returning true on success
    virtual bool openPort() = 0;

    virtual bool setBaud(int baud) = 0;

    /// Closes CM730 port
    virtual void closePort() = 0;

    /// Discards data received but not read
    virtual void clearPort() = 0;

    /// Writes up to byteCount bytes from packet to the CM730 port, returning the number of bytes written, or -1 on error
    virtual int writePort(uchar const* packet, std::size_t byteCount) = 0;

    /// Attempts to read byteCount bytes from the CM730 port into the buffer starting at packet, returning the number of bytes read, or -1 on error
    virtual int readPort(uchar* packet, std::size_t byteCount) = 0;


    virtual void lowPriorityWait() = 0;
    virtual void midPriorityWait() = 0;
    virtual void highPriorityWait() = 0;
    virtual void lowPriorityRelease() = 0;
    virtual void midPriorityRelease() = 0;
    virtual void highPriorityRelease() = 0;


    /// Sets timeout for packet receipt, called after sending a packet for which a response is expected
    virtual void setPacketTimeout(int lenPacket) = 0;

    /// Gets whether the expected packet has timed out
    virtual bool isPacketTimeout() = 0;

    /// Gets the time since the last packet was started
    virtual double getPacketTime() = 0;

    virtual unsigned long getReceivedByteCount() const = 0;
    virtual unsigned long getTransmittedByteCount() const = 0;
    virtual void resetByteCounts() = 0;

    /// Makes the calling process sleep until msec milliseconds have elapsed
    virtual void sleep(double msec) = 0;
  };
}