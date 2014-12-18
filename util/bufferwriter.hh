#pragma once

#include <cstdint>

// TODO endianness as a type param? eg: BufferWriter<LittleEndian>

// TODO write N bytes
// TODO write string of N bytes (encoding?)
// TODO test length and remaining

namespace bold
{
  class BufferWriter
  {
  public:
    BufferWriter(char* ptr)
      : d_start(ptr),
        d_ptr(ptr)
    {}

    void writeInt8(int8_t val)
    {
      *reinterpret_cast<int8_t*>(d_ptr++) = val;
    }

    void writeInt8u(uint8_t val)
    {
      *reinterpret_cast<uint8_t*>(d_ptr++) = val;
    }

    void writeInt16(int16_t val)
    {
      *reinterpret_cast<int16_t*>(d_ptr) = val;
      d_ptr += sizeof(int16_t);
    }

    void writeInt16u(uint16_t val)
    {
      *reinterpret_cast<uint16_t*>(d_ptr) = val;
      d_ptr += sizeof(uint16_t);
    }

    void writeInt32(int32_t val)
    {
      *reinterpret_cast<int32_t*>(d_ptr) = val;
      d_ptr += sizeof(int32_t);
    }

    void writeInt32u(uint32_t val)
    {
      *reinterpret_cast<uint32_t*>(d_ptr) = val;
      d_ptr += sizeof(uint32_t);
    }

    size_t pos()
    {
      return d_ptr - d_start;
    }

  private:
    const char* const d_start;
    char* d_ptr;
  };
}
