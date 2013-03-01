#include "pixellabel.hh"

uchar bold::PixelLabel::nextId = 1;

std::ostream& bold::operator<<(std::ostream &stream, bold::PixelLabel const& pixelLabel)
{
  return stream << pixelLabel.name() << " (" << (int)pixelLabel.id() << ") " << pixelLabel.hsvRange();
}