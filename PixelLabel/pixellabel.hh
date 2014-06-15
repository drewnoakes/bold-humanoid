#pragma once

#include <opencv2/core/core.hpp>
#include <string>
#include <ostream>

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

    virtual void print(std::ostream& out) const;

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

  inline void PixelLabel::print(std::ostream& out) const
  {
    out << d_name << " (" << (int)d_id << ")";
  }
}

namespace std
{
  ostream& operator<<(ostream& out, bold::PixelLabel const& PixelLabel);
}
