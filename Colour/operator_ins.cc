#include "colour.ih"

ostream& bold::operator<<(ostream &stream, Colour::bgr const& bgr)
{
  return stream <<  "B=" << (int)bgr.b
                << " G=" << (int)bgr.g
                << " R=" << (int)bgr.r;

}

ostream& bold::operator<<(ostream &stream, Colour::hsv const& hsv)
{
  return stream <<  "H=" << hsv.h
                << " S=" << hsv.s
                << " V=" << hsv.v;

}

std::ostream& bold::operator<<(std::ostream &stream, Colour::hsvRange const& hsvRange)
{
  return stream << setprecision(2)
                <<  "H=" << (int)(360*(hsvRange.h/255.0)) << "±" << (int)(360*(hsvRange.hRange/255.0))
                << " S=" << (hsvRange.s/255.0) << "±" << (hsvRange.sRange/255.0)
                << " V=" << (hsvRange.v/255.0) << "±" << (hsvRange.vRange/255.0)
                << setprecision(6);
}
