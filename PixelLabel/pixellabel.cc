#include "pixellabel.hh"

std::ostream& std::operator<<(std::ostream &stream, bold::PixelLabel const& pixelLabel)
{
  pixelLabel.print(stream);
  return stream;
}
