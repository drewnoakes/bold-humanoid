#include "cm730linux.hh"

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h>
#include <linux/serial.h>
#include <sys/ioctl.h>

#include "../../Clock/clock.hh"
#include "../../util/log.hh"

using namespace bold;
using namespace std;

CM730Linux::CM730Linux(string name)
: d_socket(-1),
  d_packetStartTimeMillis(0),
  d_packetWaitTimeMillis(0),
  d_byteTransferTimeMillis(0),
  d_txByteCount(0),
  d_rxByteCount(0)
{
  setPortName(name);
}

CM730Linux::~CM730Linux()
{
  closePort();
}

bool CM730Linux::openPort()
{
  log::verbose("CM730Linux::openPort") << "Starting";

  struct termios newtio = {0,};
  struct serial_struct serinfo = {0,};
  double baudrate = 1000000.0; //bps (1Mbps)

  closePort();

  if ((d_socket = open(d_portName.c_str(), O_RDWR|O_NOCTTY|O_NONBLOCK)) < 0)
    goto UART_OPEN_ERROR;

  // You must set 38400bps!
//   memset(&newtio, 0, sizeof(newtio));
  newtio.c_cflag      = B38400|CS8|CLOCAL|CREAD;
  newtio.c_iflag      = IGNPAR;
  newtio.c_oflag      = 0;
  newtio.c_lflag      = 0;
  newtio.c_cc[VTIME]  = 0;
  newtio.c_cc[VMIN]   = 0;
  tcsetattr(d_socket, TCSANOW, &newtio);

  // Set non-standard baudrate
  if (ioctl(d_socket, TIOCGSERIAL, &serinfo) < 0)
    goto UART_OPEN_ERROR;

  serinfo.flags &= ~ASYNC_SPD_MASK;
  serinfo.flags |= ASYNC_SPD_CUST;
  serinfo.custom_divisor = serinfo.baud_base / baudrate;

  if (ioctl(d_socket, TIOCSSERIAL, &serinfo) < 0)
    goto UART_OPEN_ERROR;

  log::info("CM730Linux::openPort") << "Opened with " << (int)baudrate << " bps baud";

  tcflush(d_socket, TCIFLUSH);

  d_byteTransferTimeMillis = (1000.0 / baudrate) * 12.0;

  return true;

UART_OPEN_ERROR:
  log::error("CM730Linux::openPort") << "Failed to open CM730 port (either the CM730 is in use by another program, or you do not have root privileges)";
  closePort();
  return false;
}

bool CM730Linux::setBaud(unsigned baud)
{
  struct serial_struct serinfo;
  int baudrate = (int)(2000000.0f / (float)(baud + 1));

  if (d_socket == -1)
    return false;

  if (ioctl(d_socket, TIOCGSERIAL, &serinfo) < 0)
  {
    fprintf(stderr, "Cannot get serial info\n");
    return false;
  }

  serinfo.flags &= ~ASYNC_SPD_MASK;
  serinfo.flags |= ASYNC_SPD_CUST;
  serinfo.custom_divisor = serinfo.baud_base / baudrate;

  if (ioctl(d_socket, TIOCSSERIAL, &serinfo) < 0)
  {
    fprintf(stderr, "Cannot set serial info\n");
    return false;
  }

  closePort();
  openPort();

  d_byteTransferTimeMillis = (float)((1000.0 / baudrate) * 12.0 * 8);

  return true;
}

bool CM730Linux::closePort()
{
  if (d_socket != -1)
  {
    if (close(d_socket) != 0)
      return false;
  }
  d_socket = -1;
  return true;
}

bool CM730Linux::clearPort()
{
  return tcflush(d_socket, TCIFLUSH) == 0;
}

bool CM730Linux::isPortOpen() const
{
  return d_socket != -1;
}

int CM730Linux::writePort(unsigned char const* packet, size_t byteCount)
{
  int i = write(d_socket, packet, byteCount);
  // errors are negative, which we don't want in the byte count
  if (i > 0)
    d_txByteCount += i;
  return i;
}

int CM730Linux::readPort(unsigned char* packet, size_t byteCount)
{
  int i = read(d_socket, packet, byteCount);
  // errors are negative, which we don't want in the byte count
  if (i > 0)
    d_rxByteCount += i;
  return i;
}

void CM730Linux::setPacketTimeout(int lenPacket)
{
  d_packetStartTimeMillis = Clock::getMillis();
  d_packetWaitTimeMillis = (d_byteTransferTimeMillis * lenPacket) + 5.0;
}

bool CM730Linux::isPacketTimeout()
{
  return getPacketTime() > d_packetWaitTimeMillis;
}

double CM730Linux::getPacketTime()
{
  double time = Clock::getMillis() - d_packetStartTimeMillis;

  if (time < 0.0)
      d_packetStartTimeMillis = Clock::getMillis();

  return time;
}

double CM730Linux::getPacketTimeoutMillis() const
{
  return d_packetWaitTimeMillis;
}

void CM730Linux::sleep(double msec)
{
  double start_time = Clock::getMillis();
  double curr_time = start_time;

  do {
    usleep((start_time + msec) - curr_time);
    curr_time = Clock::getMillis();
  } while(curr_time - start_time < msec);
}
