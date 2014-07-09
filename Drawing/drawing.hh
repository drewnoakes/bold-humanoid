#pragma once

#include "../Colour/colour.hh"
#include "../geometry/LineSegment/LineSegment2/linesegment2.hh"

#include <memory>
#include <vector>
#include <Eigen/Core>

namespace bold
{
  enum class DrawingItemType
  {
    Line = 1
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

  class Draw
  {
  public:
    static void initialise();

    static void line(Frame frame, Eigen::Vector2d const& p1, Eigen::Vector2d const& p2, Colour::bgr const& colour, double lineWidth = 1.0, double alpha = 1.0);
    static void line(Frame frame, LineSegment2d const& line, Colour::bgr const& colour, double lineWidth = 1.0, double alpha = 1.0);
    static void lineAtAngle(Frame frame, Eigen::Vector2d const& p1, double angle, double length, Colour::bgr const& colour, double lineWidth = 1.0, double alpha = 1.0);

    static void flushToStateObject();

  private:
    static std::unique_ptr<std::vector<std::unique_ptr<DrawingItem const>>> d_drawingItems;
  };
}
