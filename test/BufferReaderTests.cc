#include <gtest/gtest.h>

#include "../util/bufferreader.hh"

using namespace bold;
using namespace std;

TEST (BufferReaderTests, readInt8)
{
  vector<char> bytes = {1,2,3,4,5,6};
  BufferReader reader(bytes.data());

  EXPECT_EQ(1, reader.readInt8());
  EXPECT_EQ(2, reader.readInt8());
  EXPECT_EQ(3, reader.readInt8());
  EXPECT_EQ(4, reader.readInt8());
  EXPECT_EQ(5, reader.readInt8());
  EXPECT_EQ(6, reader.readInt8());
}

TEST (BufferReaderTests, readInt16)
{
  vector<char> bytes = {1,2,3,4,5,6};
  BufferReader reader(bytes.data());

  EXPECT_EQ(0x0201, reader.readInt16());
  EXPECT_EQ(0x0403, reader.readInt16());
  EXPECT_EQ(0x0605, reader.readInt16());
}

TEST (BufferReaderTests, readInt32)
{
  vector<char> bytes = {1,2,3,4,5,6,7,8};
  BufferReader reader(bytes.data());

  EXPECT_EQ(0x04030201, reader.readInt32());
  EXPECT_EQ(0x08070605, reader.readInt32());
}
