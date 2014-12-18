#pragma once

#include <cstdint>
#include <memory>

#include "../util/bufferreader.hh"

namespace bold
{
  class GameState;

  class GameStateDecoder
  {
  protected:
    GameStateDecoder(uint8_t version, uint32_t messageSize)
      : d_version(version),
        d_messageSize(messageSize)
    {}

  public:
    uint8_t getSupportedVersion() const { return d_version; }

    uint32_t getMessageSize() const { return d_messageSize; }

    virtual std::shared_ptr<GameState const> decode(BufferReader& reader) const = 0;

  private:
    uint8_t d_version;
    uint32_t d_messageSize;
  };
}
