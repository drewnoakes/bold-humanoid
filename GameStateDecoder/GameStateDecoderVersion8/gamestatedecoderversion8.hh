#pragma once

#include "../gamestatedecoder.hh"

namespace bold
{
  class GameStateDecoderVersion8 : public GameStateDecoder
  {
  public:
    GameStateDecoderVersion8()
      : GameStateDecoder(8, 158)
    {}

    std::shared_ptr<GameState const> decode(BufferReader& reader) const override;
  };
}
