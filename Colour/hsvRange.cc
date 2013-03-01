#include "colour.hh"

using namespace bold;
using namespace std;
using namespace cv;

std::ostream& bold::operator<<(std::ostream &stream, Colour::hsvRange const& hsvRange)
{
  return stream <<  "H=" << (int)(360*(hsvRange.h/255.0)) << "±" << (int)(360*(hsvRange.hRange/255.0))
                << " S=" << (hsvRange.s/255.0) << "±" << (hsvRange.sRange/255.0)
                << " V=" << (hsvRange.v/255.0) << "±" << (hsvRange.vRange/255.0);
}

std::ostream& bold::operator<<(std::ostream &stream, Colour::hsv const& hsv)
{
  return stream << "H=" << (int)(360*(hsv.h/255.0)) << " S=" << (hsv.s/255.0) << " V=" << (hsv.v/255.0);
}