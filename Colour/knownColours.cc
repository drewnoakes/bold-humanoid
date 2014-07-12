#include "colour.hh"

using namespace bold;
using namespace bold::Colour;

const bgr bgr::black = {0, 0, 0};
const bgr bgr::grey  = {128, 128, 128};
const bgr bgr::white = {255, 255, 255};

const bgr bgr::red        = {0, 0, 128};
const bgr bgr::lightRed   = {0, 0, 255};
const bgr bgr::darkRed    = {0, 0, 64};

const bgr bgr::green      = {0, 128, 0};
const bgr bgr::lightGreen = {0, 255, 0};
const bgr bgr::darkGreen  = {0,  64, 0};

const bgr bgr::blue       = {128, 0, 0};
const bgr bgr::lightBlue  = {255, 0, 0};
const bgr bgr::darkBlue   = {64,  0, 0};

const bgr bgr::orange = {255, 128, 0};
const bgr bgr::purple = {0x85, 0x24, 0x79}; // BH purple
const bgr bgr::yellow = {0, 255, 255};
