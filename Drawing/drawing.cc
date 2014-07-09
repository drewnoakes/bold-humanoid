#include "drawing.hh"

#include "../State/state.hh"
#include "../StateObject/DrawingState/drawingstate.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

unique_ptr<vector<unique_ptr<DrawingItem const>>> Draw::d_drawingItems;

void Draw::initialise()
{
  d_drawingItems = make_unique<vector<unique_ptr<DrawingItem const>>>();
}

void Draw::line(Frame frame, LineSegment2d const& line, Colour::bgr const& colour, double lineWidth, double alpha)
{
  Draw::line(frame, line.p1(), line.p2(), colour, lineWidth, alpha);
}

void Draw::line(Frame frame, Vector2d const& p1, Vector2d const& p2, Colour::bgr const& colour, double lineWidth, double alpha)
{
  unique_ptr<LineDrawing> line = make_unique<LineDrawing>();

  line->type = DrawingItemType::Line;
  line->frame = frame;
  line->p1 = p1;
  line->p2 = p2;
  line->colour = colour;
  line->alpha = alpha;
  line->lineWidth = lineWidth;

  d_drawingItems->push_back(std::move(line));
}

void Draw::lineAtAngle(Frame frame, Vector2d const& p1, double angle, double length, Colour::bgr const& colour, double lineWidth, double alpha)
{
  Vector2d p2 = p1;
  p2.x() += cos(angle) * length;
  p2.y() += sin(angle) * length;

  line(frame, p1, p2, colour, lineWidth, alpha);
}

void Draw::flushToStateObject()
{
  State::make<DrawingState>(std::move(d_drawingItems));

  initialise();
}
