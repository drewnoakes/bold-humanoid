#pragma once

#include "../pixellabel.hh"

namespace bold
{
  class RangePixelLabel : public PixelLabel
  {
  public:
    RangePixelLabel(std::string name, Colour::hsvRange hsvRange);

    Colour::hsvRange getHSVRange() const;
    void setHSVRange(Colour::hsvRange range);

    float labelProb(Colour::hsv const& pixelColour) const override;
    Colour::hsv modalColour() const override;

    void print(std::ostream& out);

  private:
    Colour::hsvRange d_hsvRange;
  };

  

  inline RangePixelLabel::RangePixelLabel(std::string name, Colour::hsvRange hsvRange)
    : PixelLabel{name},
    d_hsvRange{std::move(hsvRange)}
  {}
  
  Colour::hsvRange RangePixelLabel::getHSVRange() const
  {
    return d_hsvRange;
  }

  void RangePixelLabel::setHSVRange(Colour::hsvRange range)
  {
    d_hsvRange = std::move(range);
  }

  float RangePixelLabel::labelProb(Colour::hsv const& pixelColour) const
  {
    return d_hsvRange.contains(pixelColour) ? 1.0f : 0.0f;
  }

  Colour::hsv RangePixelLabel::modalColour() const
  {
    return d_hsvRange.toHsv();
  }

  void RangePixelLabel::print(std::ostream& out)
  {
    PixelLabel::print(out);
    out << " " << d_hsvRange;
  }
}
