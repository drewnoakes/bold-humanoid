#pragma once

#include "../gamestatedecoder.hh"

namespace bold
{
  class GameStateDecoderVersion7 : public GameStateDecoder
  {
  public:
    GameStateDecoderVersion7()
      : GameStateDecoder(7, 116)
    {}

    std::shared_ptr<GameState const> decode(BufferReader& reader) const override;
  };
}
