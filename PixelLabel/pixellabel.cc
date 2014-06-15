#include "pixellabel.hh"

uint8_t bold::PixelLabel::s_nextID = 1;

std::ostream& std::operator<<(std::ostream &stream, bold::PixelLabel const& pixelLabel)
{
  pixelLabel.print(stream);
  return stream;
}
