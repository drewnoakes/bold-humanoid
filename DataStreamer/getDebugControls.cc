#include "datastreamer.ih"

vector<Control> DataStreamer::getDebugControls()
{
  vector<Control> controls;

  // Image types
  vector<ControlEnumValue> imageTypes;
  imageTypes.push_back(ControlEnumValue((int)ImageType::RGB,     "RGB"));
  imageTypes.push_back(ControlEnumValue((int)ImageType::Cartoon, "Cartoon"));
  imageTypes.push_back(ControlEnumValue((int)ImageType::YCbCr,   "YCbCr"));
  imageTypes.push_back(ControlEnumValue((int)ImageType::None,    "None"));
  // TODO add/remove pass handlers depending upon the selected image type
  controls.push_back(Control::createEnum("Image", imageTypes, (int)d_imageType, [this](ControlEnumValue const& value) { d_imageType = (ImageType)value.getValue(); }));

  // Frame periods
  vector<ControlEnumValue> framePeriods;
  for (int period = 1; period <=10 ; period++)
  {
    framePeriods.push_back(ControlEnumValue(period, std::to_string(period)));
  }
  auto framePeriod = Control::createEnum("Frame period", framePeriods, d_streamFramePeriod, [this](ControlEnumValue const& value) { d_streamFramePeriod = value.getValue(); });
  framePeriod.setIsAdvanced(true);
  controls.push_back(framePeriod);

  // Head control
  auto moveHead = [](double const& pan, double const& tilt)
  {
    auto head = Head::GetInstance();
    head->m_Joint.SetEnableHeadOnly(true, true);
    head->MoveByAngleOffset(pan, tilt);
  };
  controls.push_back(Control::createAction("&blacktriangleleft;",  [&moveHead](){ moveHead( 5, 0); }));
  controls.push_back(Control::createAction("&blacktriangle;",      [&moveHead](){ moveHead( 0, 5); }));
  controls.push_back(Control::createAction("&blacktriangledown;",  [&moveHead](){ moveHead( 0,-5); }));
  controls.push_back(Control::createAction("&blacktriangleright;", [&moveHead](){ moveHead(-5, 0); }));

  // Layers
  controls.push_back(Control::createBool("Blobs",            d_drawBlobs,         [this](bool const& value) { d_drawBlobs = value; }));
  controls.push_back(Control::createBool("Line dots",        d_drawLineDots,      [this](bool const& value) { d_drawLineDots = value; }));
  controls.push_back(Control::createBool("Lines (observed)", d_drawObservedLines, [this](bool const& value) { d_drawObservedLines = value; }));
  controls.push_back(Control::createBool("Lines (expected)", d_drawExpectedLines, [this](bool const& value) { d_drawExpectedLines = value; }));

  return controls;
}
