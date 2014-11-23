#pragma once

#include "../pixellabel.hh"

namespace bold
{
  class RangePixelLabel : public PixelLabel
  {
  public:
    RangePixelLabel(std::string name, LabelClass id, Colour::hsvRange hsvRange);

    void addSample(Colour::hsv const& pixelColour) override;

    Colour::hsvRange getHSVRange() const;
    void setHSVRange(Colour::hsvRange range) override;

    float labelProb(Colour::hsv const& pixelColour) const override;
    Colour::hsv modalColour() const override;

    void print(std::ostream& out) const override;

  private:
    Colour::hsvRange d_hsvRange;
  };

  inline RangePixelLabel::RangePixelLabel(std::string name, LabelClass id, Colour::hsvRange hsvRange)
    : PixelLabel{name, id, hsvRange.toBgr()},
      d_hsvRange{std::move(hsvRange)}
  {}

  inline void RangePixelLabel::addSample(Colour::hsv const& pixelColour)
  {
    d_hsvRange = d_hsvRange.containing(pixelColour);
  }
  
  inline Colour::hsvRange RangePixelLabel::getHSVRange() const
  {
    return d_hsvRange;
  }

  inline void RangePixelLabel::setHSVRange(Colour::hsvRange range)
  {
    d_hsvRange = std::move(range);
  }

  inline float RangePixelLabel::labelProb(Colour::hsv const& pixelColour) const
  {
    return d_hsvRange.contains(pixelColour) ? 1.0f : 0.0f;
  }

  inline Colour::hsv RangePixelLabel::modalColour() const
  {
    return d_hsvRange.toHsv();
  }

  inline void RangePixelLabel::print(std::ostream& out) const
  {
    PixelLabel::print(out);
    out << " " << d_hsvRange;
  }
}
