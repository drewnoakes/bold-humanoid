#pragma once

#include <cstdint>

// TODO endianness as a type param? eg: BufferReader<LittleEndian>

// TODO read N bytes
// TODO read string of N bytes (encoding?)
// TODO test length and remaining

namespace bold
{
  class BufferReader
  {
  public:
    BufferReader(const char* ptr)
      : d_start(ptr),
        d_ptr(ptr)
    {}

    int8_t readInt8()
    {
      return static_cast<int8_t>(*d_ptr++);
    }

    uint8_t readInt8u()
    {
      return static_cast<uint8_t>(*d_ptr++);
    }

    int16_t readInt16()
    {
      auto val = *reinterpret_cast<const int16_t*>(d_ptr);
      d_ptr += sizeof(int16_t);
      return val;
    }

    uint16_t readInt16u()
    {
      auto val = *reinterpret_cast<const uint16_t*>(d_ptr);
      d_ptr += sizeof(uint16_t);
      return val;
    }

    int32_t readInt32()
    {
      auto val = *reinterpret_cast<const int32_t*>(d_ptr);
      d_ptr += sizeof(int32_t);
      return val;
    }

    uint32_t readInt32u()
    {
      auto val = *reinterpret_cast<const uint32_t*>(d_ptr);
      d_ptr += sizeof(uint32_t);
      return val;
    }

    size_t pos()
    {
      return d_ptr - d_start;
    }

  private:
    const char* const d_start;
    const char* d_ptr;
  };
}
