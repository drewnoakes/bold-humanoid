#include "visualcortex.ih"

VisualCortex::VisualCortex()
  : d_minBallArea(8*8),
    d_isBallVisible(false),
    d_lineFinder(nullptr),
    d_imagePasser(nullptr),
    d_blobDetectPass(nullptr),
    d_cartoonPass(nullptr),
    d_labelCountPass(nullptr),
    d_lineDotPass(nullptr),
    d_pixelLabelById(),
    d_observations(),
    d_goalObservations(),
    d_ballObservation()
{}
