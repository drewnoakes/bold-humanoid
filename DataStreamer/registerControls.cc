#include "datastreamer.ih"

void DataStreamer::registerControls(string family, vector<Control> controls)
{
  cout << "[DataStreamer:registerControls] Adding control family '" << family << "'" << endl;

  if (d_controlsByIdByFamily.find(family) != d_controlsByIdByFamily.end())
  {
    cerr << "[DataStreamer:registerControls] [WARNING] Controls already exist for this family" << endl;
  }

  map<unsigned,Control> controlsById;
  for (Control const& control : controls)
  {
    controlsById[control.getId()] = control;
  }

  d_controlsByIdByFamily[family] = controlsById;
}
