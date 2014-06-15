#pragma once

#include <opencv2/core/core.hpp>
#include <string>

#include "../Colour/colour.hh"
#include "../util/assert.hh"

namespace bold
{
  class PixelLabel
  {
  public:
    PixelLabel(std::string name);

    uint8_t getID() const;
    std::string getName() const;

    virtual float labelProb(Colour::hsv const& pixelColour) const = 0;
    virtual Colour::hsv modalColour() const = 0;

  private:
    uint8_t d_id;
    std::string d_name;

    static uint8_t s_nextID;
  };



  inline PixelLabel::PixelLabel(std::string name)
    : d_id{s_nextID++},
    d_name{std::move(name)}
  {}

  inline uint8_t PixelLabel::getID() const
  {
    return d_id;
  }

  inline std::string PixelLabel::getName() const
  {
    return d_name;
  }

}
