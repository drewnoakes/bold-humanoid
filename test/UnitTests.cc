#include "gtest/gtest.h"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <cmath>

#include "../Geometry/geometry.hh"
#include "../CameraModel/cameramodel.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

// TODO move to CMake and split this up into one file per class-under-test

//PrintTo(const T&, ostream*)

ostream& operator<<(ostream& stream, Eigen::Vector2i const& v)
{
  return stream << "(" << v.x() << ", " << v.y() << ")";
}

bool operator==(Eigen::Vector2i const& a, Eigen::Vector2i const& b)
{
  return a.x() == b.x() && a.y() == b.y();
}

//////////////////////////////////////////////////////////////////////////////

TEST (Bounds2iTests, contains)
{
  EXPECT_TRUE  ( Bounds2i(0, 0, 10, 10).contains( Vector2i(5, 5) ) );
  EXPECT_TRUE  ( Bounds2i(0, 0, 10, 10).contains( Vector2i(0, 0) ) );
  EXPECT_TRUE  ( Bounds2i(0, 0, 10, 10).contains( Vector2i(10, 10) ) );

  EXPECT_FALSE ( Bounds2i(0, 0, 10, 10).contains( Vector2i(15, 15) ) );
  EXPECT_FALSE ( Bounds2i(0, 0, 10, 10).contains( Vector2i(-1, -1) ) );
  EXPECT_FALSE ( Bounds2i(0, 0, 10, 10).contains( Vector2i(-1, 5) ) );
  EXPECT_FALSE ( Bounds2i(0, 0, 10, 10).contains( Vector2i(5, 15) ) );
}

//////////////////////////////////////////////////////////////////////////////

TEST (LineSegmentTests, delta)
{
  EXPECT_EQ (Vector2i(50,100),  LineSegment2i(Vector2i(50,0),  Vector2i(100,100)).delta());
  EXPECT_EQ (Vector2i(-10,-20), LineSegment2i(Vector2i(40,30), Vector2i(30,10)).delta());
  EXPECT_EQ (Vector2i(2,1),     LineSegment2i(Vector2i(1,0),   Vector2i(3,1)).delta());
}

TEST (LineSegmentTests, points_not_identical)
{
  ASSERT_THROW (LineSegment2i(Vector2i(50,0), Vector2i(50,0)), std::string);
}

TEST (LineSegmentTests, gradient)
{
  EXPECT_EQ (0,   LineSegment2i(Vector2i(0,0), Vector2i(100,0)).gradient());
  EXPECT_EQ (1,   LineSegment2i(Vector2i(0,0), Vector2i(10,10)).gradient());
  EXPECT_EQ (-1,  LineSegment2i(Vector2i(0,0), Vector2i(10,-10)).gradient());
  EXPECT_EQ (0.5, LineSegment2i(Vector2i(1,0), Vector2i(3,1)).gradient());

  // Vertical lines have infinite gradient
  EXPECT_EQ (FP_INFINITE, LineSegment2i(Vector2i(0,0), Vector2i(0,10)).gradient());
}

TEST (LineSegmentTests, yIntersection)
{
  EXPECT_EQ (0,    LineSegment2i(Vector2i(0,0),   Vector2i(100,0)).yIntersection());
  EXPECT_EQ (10,   LineSegment2i(Vector2i(10,10), Vector2i(20,10)).yIntersection());
  EXPECT_EQ (-1,   LineSegment2i(Vector2i(1,0),   Vector2i(2,1)).yIntersection());
  EXPECT_EQ (-0.5, LineSegment2i(Vector2i(1,0),   Vector2i(3,1)).yIntersection());

  // Vertical lines have no yIntersection
  EXPECT_EQ (FP_NAN, LineSegment2i(Vector2i(10,0), Vector2i(10,10)).yIntersection());
}

TEST (LineSegmentTests, angle)
{
  EXPECT_EQ (0,         LineSegment2i(Vector2i(0,0), Vector2i(1,0)).angle());
  EXPECT_EQ (M_PI/4,    LineSegment2i(Vector2i(0,0), Vector2i(1,1)).angle());
  EXPECT_EQ (M_PI/2,    LineSegment2i(Vector2i(1,0), Vector2i(1,1)).angle());
  EXPECT_EQ (M_PI,      LineSegment2i(Vector2i(1,1), Vector2i(0,1)).angle());
  EXPECT_EQ (atan(0.5), LineSegment2i(Vector2i(1,0), Vector2i(3,1)).angle());
}

TEST (LineSegmentTests, cropTo)
{
  // Completely inside
  EXPECT_EQ( LineSegment2i(100, 100, 200, 200), LineSegment2i(100, 100, 200, 200).cropTo(Bounds2i(0, 0, 300, 300)) );

  // Completely outside
  EXPECT_EQ( LineSegment2i(100, 100, 200, 200), LineSegment2i(100, 100, 200, 200).cropTo(Bounds2i(0, 0, 300, 300)) );

  // Spanning right edge
  EXPECT_EQ( LineSegment2i(100, 100, 200, 200), LineSegment2i(100, 100, 300, 300).cropTo(Bounds2i(0, 0, 200, 200)) );
}

TEST (LineSegmentTests, tryIntersect)
{
  // Perpendicular
  EXPECT_EQ( Maybe<Vector2i>(Vector2i(1, 1)), LineSegment2i(1, 0, 1, 2).tryIntersect(LineSegment2i(0, 1, 2, 1)) );

  // Perpendicular
  EXPECT_EQ( Maybe<Vector2i>(Vector2i(5, 5)), LineSegment2i(0, 10, 10, 0).tryIntersect(LineSegment2i(0, 0, 10, 10)) );

  // Touching (V shape)
  EXPECT_EQ( Maybe<Vector2i>(Vector2i(0, 0)), LineSegment2i(-1, 1, 0, 0).tryIntersect(LineSegment2i(0, 0, 1, 1)) );

  // Touching (T shape)
  EXPECT_EQ( Maybe<Vector2i>(Vector2i(0, 1)), LineSegment2i(-1, 1, 1, 1).tryIntersect(LineSegment2i(0, 0, 0, 1)) );
  EXPECT_EQ( Maybe<Vector2i>(Vector2i(1, 1)), LineSegment2i(0, 1, 2, 1).tryIntersect(LineSegment2i(1, 1, 1, 2)) );

  // Touching (L shape)
  EXPECT_EQ( Maybe<Vector2i>(Vector2i(-1, 1)), LineSegment2i(-2, 1, -1, 1).tryIntersect(LineSegment2i(-1, 1, -1, 2)) );
  EXPECT_EQ( Maybe<Vector2i>(Vector2i(-1, 1)), LineSegment2i(-1, 1, -2, 1).tryIntersect(LineSegment2i(-1, 2, -1, 1)) );
  EXPECT_EQ( Maybe<Vector2i>(Vector2i(-1, 1)), LineSegment2i(-2, 1, -1, 1).tryIntersect(LineSegment2i(-1, 2, -1, 1)) );
  EXPECT_EQ( Maybe<Vector2i>(Vector2i(1, 1)), LineSegment2i(1, 1, 1, 2).tryIntersect(LineSegment2i(1, 1, 2, 1)) );
  EXPECT_EQ( Maybe<Vector2i>(Vector2i(0, 0)), LineSegment2i(0, 0, 0, 1).tryIntersect(LineSegment2i(0, 0, 1, 0)) );

  // Parallel (vertical)
  EXPECT_EQ( Maybe<Vector2i>::empty(), LineSegment2i(1, 1, 1, 2).tryIntersect(LineSegment2i(2, 0, 2, 1)) );
  EXPECT_EQ( Maybe<Vector2i>::empty(), LineSegment2i(1, 1, 1, 2).tryIntersect(LineSegment2i(2, 0, 2, 1)) );

  // Parallel (horizontal)
  EXPECT_EQ( Maybe<Vector2i>::empty(), LineSegment2i(0, 0, 1, 0).tryIntersect(LineSegment2i(0, 1, 1, 1)) );

  // Colinear
  EXPECT_EQ( Maybe<Vector2i>::empty(), LineSegment2i(0, 0, 1, 0).tryIntersect(LineSegment2i(2, 0, 3, 0)) );
  EXPECT_EQ( Maybe<Vector2i>::empty(), LineSegment2i(1, 1, 2, 2).tryIntersect(LineSegment2i(3, 3, 4, 4)) );
  // this case doesn't work -- is it important?
//EXPECT_EQ( Maybe<Vector2i>(Vector2i(2,2)), LineSegment2i(1, 1, 2, 2).tryIntersect(LineSegment2i(2, 2, 3, 3)) );
}

TEST (DISABLED_LineSegmentTests, toLine)
{
  // Vertical line at x=10
  EXPECT_EQ (Line(10, 0), LineSegment2i(Vector2i(10,0), Vector2i(10,10)).toLine());

  // Vertical line at x=-10
  EXPECT_EQ (Line(-10, 0), LineSegment2i(Vector2i(-10,0), Vector2i(-10,10)).toLine());

  // Horizontal line at y=10
  EXPECT_EQ (Line(10, M_PI/2), LineSegment2i(Vector2i(0,10), Vector2i(10,10)).toLine());

  // Vertical line at x=-10
  EXPECT_EQ (Line(-10, M_PI/2), LineSegment2i(Vector2i(0,-10), Vector2i(10,-10)).toLine());

  // y = x
  EXPECT_EQ (Line(0, M_PI*3/4), LineSegment2i(Vector2i(0,0), Vector2i(1,1)).toLine());

  // y = -x
  EXPECT_EQ (Line(0, M_PI/4), LineSegment2i(Vector2i(0,0), Vector2i(1,-1)).toLine());

  // line from (1,0) to (2,1)
  EXPECT_EQ (Line(-sqrt(2)/2, M_PI*3/4), LineSegment2i(Vector2i(1,0), Vector2i(2,1)).toLine());

  // line from (1,0) to (3,1)
  EXPECT_EQ (Line(-sin(atan(0.5)), M_PI/2+atan(0.5)), LineSegment2i(Vector2i(1,0), Vector2i(3,1)).toLine());

  // line from (3,1) to (1,0) -- same as prior test, but in reverse order (shouldn't matter)
  EXPECT_EQ (Line(-sin(atan(0.5)), M_PI/2+atan(0.5)), LineSegment2i(Vector2i(3,1), Vector2i(1,0)).toLine());

  // line from (5.5,2) to (3,3.5)
  EXPECT_EQ (Line(-sin(atan(0.5)), M_PI+atan(-0.6)), LineSegment2i(Vector2i(5.5,2), Vector2i(3,3.5)).toLine());
}

//////////////////////////////////////////////////////////////////////////////

// TEST (CameraProjectionTests, design)
// {
//   vector<LineSegment3d> worldLines = {
//     LineSegment3d(Vector3d( 1, 1, 0), Vector3d( 1,-1, 0)),
//     LineSegment3d(Vector3d( 1,-1, 0), Vector3d(-1,-1, 0)),
//     LineSegment3d(Vector3d(-1,-1, 0), Vector3d(-1, 1, 0)),
//     LineSegment3d(Vector3d(-1, 1, 0), Vector3d( 1, 1, 0))
//   };
//
//   Camera camera();
//
//   // TODO set up camera position/orientation/fov/focal-length
//
//   Matrix worldToCamera;
//
//   vector<LineSegment2i> screenLines = camera.project(worldLines, worldToCamera);
// }

TEST (CameraProjectionTests, getProjector)
{
  auto imageWidth = 640;
  auto imageHeight = 480;
  auto focalLength = 1;
  auto rangeVertical = (46.0/180) * M_PI; // 46 degrees from top to bottom
  auto rangeHorizontal = (60.0/180) * M_PI; // 60 degrees from left to right

  CameraModel cameraModel(imageWidth, imageHeight, focalLength, rangeVertical, rangeHorizontal);

  // From 3D camera space to 2D screen space
  Affine3d projectionTransform = cameraModel.getProjectionTransform();

  EXPECT_EQ( Vector3d(0,0,0), projectionTransform.translation() ) << "No translation in projection matrix";

  std::function<Vector2i(Vector3d const&)> projector = cameraModel.getProjector();

  double projectionPlaneWidth = atan(cameraModel.rangeHorizontal() / 2) * cameraModel.focalLength() * 2;
  double pixelwidth = projectionPlaneWidth / cameraModel.imageWidth();

  EXPECT_EQ( Vector2i(0,0), projector(Vector3d(0,0,10)) );
  EXPECT_EQ( Vector2i(10/pixelwidth,0), projector(Vector3d(10,0,cameraModel.focalLength())) );
}

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}



























