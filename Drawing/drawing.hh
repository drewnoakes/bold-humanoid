#pragma once

#include "../Colour/colour.hh"
#include "../geometry/LineSegment/LineSegment2/linesegment2.hh"
#include "../geometry/Polygon2.hh"

#include <memory>
#include <vector>
#include <Eigen/Core>

namespace bold
{
  enum class DrawingItemType
  {
    Line = 1,
    Circle = 2,
    Polygon = 3,
  };

  enum class Frame
  {
    Camera = 1,
    Agent = 2,
    World = 3
  };

  struct DrawingItem
  {
    Frame frame;
    DrawingItemType type;
  };

  struct LineDrawing : public DrawingItem
  {
    Eigen::Vector2d p1;
    Eigen::Vector2d p2;
    Colour::bgr colour;
    double lineWidth;
    double alpha;
  };

  struct CircleDrawing : public DrawingItem
  {
    Eigen::Vector2d centre;
    double radius;
    Colour::bgr fillColour;
    Colour::bgr strokeColour;
    double fillAlpha;
    double strokeAlpha;
    double lineWidth;
  };

  struct PolygonDrawing : public DrawingItem
  {
    PolygonDrawing(Polygon2d const& polygon)
      : polygon(polygon)
    {}

    Polygon2d polygon;
    Colour::bgr fillColour;
    Colour::bgr strokeColour;
    double fillAlpha;
    double strokeAlpha;
    double lineWidth;
  };

  class Draw
  {
  public:
    static void initialise();

    static void circle(Frame frame, Eigen::Vector2d const& centre, double radius, Colour::bgr const& colour, double alpha = 1.0, double lineWidth = 1.0);
    static void fillCircle(Frame frame, Eigen::Vector2d const& centre, double radius, Colour::bgr const& fillColour, double fillAlpha, Colour::bgr const& strokeColour, double strokeAlpha, double lineWidth = 1.0);

    static void polygon(Frame frame, Polygon2d const& polygon, Colour::bgr const& strokeColour, double strokeAlpha, double lineWidth = 1.0);
    static void fillPolygon(Frame frame, Polygon2d const& polygon, Colour::bgr const& fillColour, double fillAlpha, Colour::bgr const& strokeColour, double strokeAlpha, double lineWidth = 1.0);

    static void line(Frame frame, Eigen::Vector2d const& p1, Eigen::Vector2d const& p2, Colour::bgr const& colour, double alpha = 1.0, double lineWidth = 1.0);
    static void line(Frame frame, LineSegment2d const& line, Colour::bgr const& colour, double alpha = 1.0, double lineWidth = 1.0);
    static void lineAtAngle(Frame frame, Eigen::Vector2d const& p1, double angle, double length, Colour::bgr const& colour, double alpha = 1.0, double lineWidth = 1.0);

    static void flushToStateObject();

  private:
    static std::unique_ptr<std::vector<std::unique_ptr<DrawingItem const>>> d_drawingItems;
  };
}
