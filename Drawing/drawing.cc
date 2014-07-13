#include "drawing.hh"

#include "../State/state.hh"
#include "../StateObject/DrawingState/drawingstate.hh"

using namespace bold;
using namespace bold::Colour;
using namespace Eigen;
using namespace std;

unique_ptr<vector<unique_ptr<DrawingItem const>>> Draw::d_drawingItems;

void Draw::initialise()
{
  d_drawingItems = make_unique<vector<unique_ptr<DrawingItem const>>>();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Draw::line(Frame frame, LineSegment2d const& line, bgr const& colour, double lineWidth, double alpha)
{
  Draw::line(frame, line.p1(), line.p2(), colour, lineWidth, alpha);
}

void Draw::line(Frame frame, Vector2d const& p1, Vector2d const& p2, bgr const& colour, double lineWidth, double alpha)
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

void Draw::lineAtAngle(Frame frame, Vector2d const& p1, double angle, double length, bgr const& colour, double lineWidth, double alpha)
{
  Vector2d p2 = p1;
  p2.x() += cos(angle) * length;
  p2.y() += sin(angle) * length;

  line(frame, p1, p2, colour, lineWidth, alpha);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Draw::circle(Frame frame, Eigen::Vector2d const& centre, double radius, bgr const& strokeColour, double strokeAlpha, double lineWidth)
{
  fillCircle(frame, centre, radius, bgr::black, 0.0, strokeColour, strokeAlpha, lineWidth);
}

void Draw::fillCircle(Frame frame, Eigen::Vector2d const& centre, double radius, bgr const& fillColour, double fillAlpha, bgr const& strokeColour, double strokeAlpha, double lineWidth)
{
  unique_ptr<CircleDrawing> circle = make_unique<CircleDrawing>();

  circle->type = DrawingItemType::Circle;
  circle->frame = frame;
  circle->centre = centre;
  circle->radius = radius;
  circle->strokeColour = strokeColour;
  circle->strokeAlpha = strokeAlpha;
  circle->fillColour = fillColour;
  circle->fillAlpha = fillAlpha;
  circle->lineWidth = lineWidth;

  d_drawingItems->push_back(std::move(circle));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Draw::polygon(Frame frame, Polygon2d const& polygon, Colour::bgr const& strokeColour, double strokeAlpha, double lineWidth)
{
  fillPolygon(frame, polygon, bgr::black, 0.0, strokeColour, strokeAlpha, lineWidth);
}

void Draw::fillPolygon(Frame frame, Polygon2d const& polygon, bgr const& fillColour, double fillAlpha, bgr const& strokeColour, double strokeAlpha, double lineWidth)
{
  unique_ptr<PolygonDrawing> poly = make_unique<PolygonDrawing>(polygon);

  poly->type = DrawingItemType::Polygon;
  poly->frame = frame;
  poly->fillColour = fillColour;
  poly->strokeColour = strokeColour;
  poly->fillAlpha = fillAlpha;
  poly->strokeAlpha = strokeAlpha;
  poly->lineWidth = lineWidth;

  d_drawingItems->push_back(std::move(poly));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Draw::flushToStateObject()
{
  State::make<DrawingState>(std::move(d_drawingItems));

  initialise();
}
