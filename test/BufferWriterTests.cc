#include <gtest/gtest.h>

#include "../util/bufferwriter.hh"

using namespace bold;
using namespace std;

void testEqual(const char* buffer, vector<char> expected)
{
  for (char i = 0; i < expected.size(); i++)
    EXPECT_EQ(expected[i], buffer[i]);
}

TEST (BufferWriterTests, writeInt8)
{
  char buffer[6];
  BufferWriter writer(buffer);

  writer.writeInt8(1);
  writer.writeInt8(2);
  writer.writeInt8(3);
  writer.writeInt8u(4);
  writer.writeInt8u(5);
  writer.writeInt8u(6);

  testEqual(buffer, {1, 2, 3, 4, 5, 6});
}

TEST (BufferWriterTests, writeInt16)
{
  char buffer[6];
  BufferWriter writer(buffer);

  writer.writeInt16(0x0201);
  writer.writeInt16(0x0403);
  writer.writeInt16(0x0605);

  testEqual(buffer, {1, 2, 3, 4, 5, 6});
}

TEST (BufferWriterTests, writeInt32)
{
  char buffer[8];
  BufferWriter writer(buffer);

  writer.writeInt32(0x04030201);
  writer.writeInt32(0x08070605);

  testEqual(buffer, {1, 2, 3, 4, 5, 6, 7, 8});
}
