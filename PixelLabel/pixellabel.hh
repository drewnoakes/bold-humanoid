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
    PixelLabel(std::string name, LabelClass id, Colour::bgr themeColour);

    LabelClass getID() const { return d_id; }
    std::string getName() const { return d_name; }
    Colour::bgr getThemeColour() const { return d_themeColour; }

    virtual void setHSVRange(Colour::hsvRange range) {}
    virtual void addSample(Colour::hsv const& pixelColour) = 0;
    virtual float labelProb(Colour::hsv const& pixelColour) const = 0;
    virtual Colour::hsv modalColour() const = 0;

    virtual void print(std::ostream& out) const;

  private:
    LabelClass d_id;
    std::string d_name;
    Colour::bgr d_themeColour;
  };

  inline PixelLabel::PixelLabel(std::string name, LabelClass id, Colour::bgr themeColour)
    : d_id(id),
      d_name(name),
      d_themeColour(themeColour)
  {}

  inline void PixelLabel::print(std::ostream& out) const
  {
    out << d_name << " (" << (int)d_id << ")";
  }
}

namespace std
{
  ostream& operator<<(ostream& out, bold::PixelLabel const& PixelLabel);
}
