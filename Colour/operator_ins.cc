#include "colour.ih"

ostream& bold::operator<<(ostream &stream, Colour::bgr const& bgr)
{
  return stream <<  "B=" << (unsigned int)bgr.b
                << " G=" << (unsigned int)bgr.g
                << " R=" << (unsigned int)bgr.r;
}

ostream& bold::operator<<(ostream &stream, Colour::hsv const& hsv)
{
  return stream <<  "H=" << (unsigned int)hsv.h
                << " S=" << (unsigned int)hsv.s
                << " V=" << (unsigned int)hsv.v;
}

ostream& bold::operator<<(ostream &stream, Colour::hsvRange const& hsvRange)
{
  return stream << setprecision(2)
                <<  "H=" << (int)(360*(hsvRange.hMin/255.0)) << "->" << (int)(360*(hsvRange.hMax/255.0))
                << " S=" << (hsvRange.sMin/255.0) << "->" << (hsvRange.sMax/255.0)
                << " V=" << (hsvRange.vMin/255.0) << "->" << (hsvRange.vMax/255.0)
                << setprecision(6);
}
