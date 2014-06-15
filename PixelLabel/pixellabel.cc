#include "pixellabel.hh"

uint8_t bold::PixelLabel::s_nextID = 1;

std::ostream& bold::operator<<(std::ostream &stream, bold::PixelLabel const& pixelLabel)
{
  return stream << pixelLabel.name() << " (" << (int)pixelLabel.id() << ") " << pixelLabel.hsvRange();
}
