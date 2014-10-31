#pragma once

#include <opencv2/core/core.hpp>
#include <string>
#include <ostream>

#include "../Colour/colour.hh"
#include "../util/assert.hh"

namespace bold
{
  enum class LabelClass : uint8_t
  {
    GOAL = 1,
    BALL,
    FIELD,
    LINE,
    CYAN,
    MAGENTA,
    BORDER
  };

  class PixelLabel
  {
  public:
    PixelLabel(std::string name, LabelClass id);

    LabelClass getID() const;
    std::string getName() const;

    virtual void addSample(Colour::hsv const& pixelColour) = 0;
    virtual float labelProb(Colour::hsv const& pixelColour) const = 0;
    virtual Colour::hsv modalColour() const = 0;

    virtual void print(std::ostream& out) const;

  private:
    LabelClass d_id;
    std::string d_name;
  };

  inline PixelLabel::PixelLabel(std::string name, LabelClass id)
    : d_id{id},
    d_name{std::move(name)}
  {}

  inline LabelClass PixelLabel::getID() const
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
