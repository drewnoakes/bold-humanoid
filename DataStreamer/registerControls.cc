#include "datastreamer.ih"

void DataStreamer::registerControls(string family, vector<std::shared_ptr<Control const>> controls)
{
  cout << "[DataStreamer:registerControls] Adding control family '" << family << "'" << endl;

  if (d_controlsByIdByFamily.find(family) != d_controlsByIdByFamily.end())
    cerr << "[DataStreamer:registerControls] [WARNING] Controls already exist for family: " << family << endl;

  map<unsigned, shared_ptr<Control const>> controlsById;

  for (auto control : controls)
    controlsById[control->getId()] = control;

  d_controlsByIdByFamily[family] = controlsById;
}
