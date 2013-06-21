#include "gtest/gtest.h"
#include "helpers.hh"
#include "../UDPSocket/udpsocket.hh"
#include "../util/Maybe.hh"

#include <sys/socket.h>

TEST (UDPSocketTests, DISABLED_communication)
{
  int receiverPort = 8765;
  int senderPort = 5678;

  UDPSocket sender;
  EXPECT_TRUE(sender.setBlocking(false));
  EXPECT_TRUE(sender.setBroadcast(true));
  EXPECT_TRUE(sender.setTarget("255.255.255.255", receiverPort));
  EXPECT_TRUE(sender.bind("", senderPort));

  UDPSocket receiver;
  EXPECT_TRUE(receiver.setBlocking(true));
  EXPECT_TRUE(receiver.setMulticastLoopback(true));
  EXPECT_TRUE(receiver.bind("", receiverPort));

  EXPECT_TRUE(sender.send("Hello"));

  sockaddr_in from;
  int len = sizeof(from);
  char packet[100] = {0};
  int bytesRead = receiver.receiveFrom(packet, sizeof(packet), &from, &len);

  EXPECT_EQ(string("Hello").length(), bytesRead);
  EXPECT_EQ("Hello", string(packet));

  // use 'from' to send a message back to the originator
  receiver.setTarget(from);

  sender.setBlocking(true);

  EXPECT_TRUE(receiver.send("Hello yourself"));

  memset(packet, 0, sizeof(packet));

  bytesRead = sender.receive(packet, sizeof(packet));

  EXPECT_EQ(string("Hello yourself").length(), bytesRead);
  EXPECT_EQ("Hello yourself", string(packet));
}

TEST (UDPSocketTests, loopback)
{
  int port = 8765;

  UDPSocket socket;

  EXPECT_TRUE(socket.setBlocking(true));
  EXPECT_TRUE(socket.setBroadcast(true));
  EXPECT_TRUE(socket.setMulticastLoopback(true));
  EXPECT_TRUE(socket.setTarget("255.255.255.255", port));
  EXPECT_TRUE(socket.bind("", port));

  EXPECT_TRUE(socket.send("Hello"));

  char packet[100] = {0};
  int bytesRead = socket.receive(packet, sizeof(packet));

  EXPECT_EQ(string("Hello").length(), bytesRead);
  EXPECT_EQ("Hello", string(packet));
}
