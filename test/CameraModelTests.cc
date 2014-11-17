#include <gtest/gtest.h>
#include "helpers.hh"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "../CameraModel/cameramodel.hh"
#include "../geometry/Bounds.hh"
#include "../geometry/LineSegment/LineSegment2/LineSegment2i/linesegment2i.hh"

using namespace bold;
using namespace Eigen;

TEST (CameraModelTests, directionForPixel)
{
  auto imageWidth = 11;
  auto imageHeight = 11;
  auto rangeVerticalDegs = 90;
  auto rangeHorizontalDegs = 90;

  CameraModel cameraModel1(imageWidth, imageHeight,
                           rangeVerticalDegs, rangeHorizontalDegs);

  EXPECT_TRUE ( VectorsEqual(Vector3d(0, 1, 0).normalized(),
                             cameraModel1.directionForPixel(Vector2d(5.5, 5.5)) ) );

  EXPECT_TRUE ( VectorsEqual(Vector3d(1, 1, 0).normalized(),
                             cameraModel1.directionForPixel(Vector2d( 0, 5.5)) ) );
  EXPECT_TRUE ( VectorsEqual(Vector3d(-1, 1, 0).normalized(),
                             cameraModel1.directionForPixel(Vector2d(11, 5.5)) ) );

  EXPECT_TRUE ( VectorsEqual(Vector3d(0, 1, -1).normalized(),
                             cameraModel1.directionForPixel(Vector2d(5.5,  0)) ) );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0, 1,  1).normalized(),
                             cameraModel1.directionForPixel(Vector2d(5.5, 11)) ) );

  auto v = Vector3d( -1, 1, 0).normalized();
  v.x() *= 2.0/5.5;
  EXPECT_TRUE ( VectorsEqual(v.normalized(),
                             cameraModel1.directionForPixel(Vector2d(7.5,  5.5)) ) );

  rangeVerticalDegs = 45;
  rangeHorizontalDegs = 60;

  double th = tan(.5 * 60.0 / 180.0 * M_PI);
  double tv = tan(.5 * 45.0 / 180.0 * M_PI);

  CameraModel cameraModel2(imageWidth, imageHeight,
                           rangeVerticalDegs, rangeHorizontalDegs);

  EXPECT_TRUE ( VectorsEqual(Vector3d(0, 1, 0).normalized(),
                             cameraModel2.directionForPixel(Vector2d(5.5, 5.5)) ) );

  EXPECT_TRUE ( VectorsEqual(Vector3d(th, 1, 0).normalized(),
                             cameraModel2.directionForPixel(Vector2d( 0, 5.5)) ) );
  EXPECT_TRUE ( VectorsEqual(Vector3d(-th, 1, 0).normalized(),
                             cameraModel2.directionForPixel(Vector2d(11, 5.5)) ) );

  EXPECT_TRUE ( VectorsEqual(Vector3d(0, 1, -tv).normalized(),
                             cameraModel2.directionForPixel(Vector2d(5.5,  0)) ) );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0, 1,  tv).normalized(),
                             cameraModel2.directionForPixel(Vector2d(5.5, 11)) ) );

}

TEST (CameraModelTests, pixelForDirection)
{
  auto imageWidth = 11;
  auto imageHeight = 11;
  auto rangeVerticalDegs = 90;
  auto rangeHorizontalDegs = 90;

  CameraModel cameraModel1(imageWidth, imageHeight, rangeVerticalDegs, rangeHorizontalDegs);

  EXPECT_EQ ( Maybe<Vector2d>::empty(), cameraModel1.pixelForDirection(Vector3d(0,  0, 0)) );
  EXPECT_EQ ( Maybe<Vector2d>::empty(), cameraModel1.pixelForDirection(Vector3d(1,  0, 0)) );
  EXPECT_EQ ( Maybe<Vector2d>::empty(), cameraModel1.pixelForDirection(Vector3d(0,  0, 1)) );
  EXPECT_EQ ( Maybe<Vector2d>::empty(), cameraModel1.pixelForDirection(Vector3d(1, -1, 1)) );

  EXPECT_TRUE ( VectorsEqual( Vector2d(5.5, 5.5), *cameraModel1.pixelForDirection(Vector3d(0, 1, 0)) ) );

  EXPECT_TRUE ( VectorsEqual( Vector2d( 0, 5.5), *cameraModel1.pixelForDirection(Vector3d( 1, 1, 0)) ) );
  EXPECT_TRUE ( VectorsEqual( Vector2d(11, 5.5), *cameraModel1.pixelForDirection(Vector3d(-1, 1, 0)) ) );

  EXPECT_TRUE ( VectorsEqual( Vector2d(5.5,  0), *cameraModel1.pixelForDirection(Vector3d(0, 1, -1)) ) );
  EXPECT_TRUE ( VectorsEqual( Vector2d(5.5, 11), *cameraModel1.pixelForDirection(Vector3d(0, 2,  2)) ) );

  auto v = Vector3d(-1, 1, 0).normalized();
  v.x() *= 2.0/5.5;
  EXPECT_TRUE ( VectorsEqual( Vector2d(7.5, 5.5), *cameraModel1.pixelForDirection(v) ) );

  // 2 time range
  rangeVerticalDegs = 45;
  rangeHorizontalDegs = 60;

  CameraModel cameraModel2(imageWidth, imageHeight, rangeVerticalDegs, rangeHorizontalDegs);

  double th = tan(.5 * 60.0 / 180.0 * M_PI);
  double tv = tan(.5 * 45.0 / 180.0 * M_PI);

  EXPECT_TRUE ( VectorsEqual( Vector2d(5.5, 5.5), *cameraModel2.pixelForDirection(Vector3d(0, 1, 0)) ) );

  EXPECT_TRUE ( VectorsEqual( Vector2d( 0, 5.5), *cameraModel2.pixelForDirection(Vector3d(th, 1, 0)) ) );
  EXPECT_TRUE ( VectorsEqual( Vector2d(11, 5.5), *cameraModel2.pixelForDirection(Vector3d(-th, 1, 0)) ) );

  EXPECT_TRUE ( VectorsEqual( Vector2d(5.5,  0), *cameraModel2.pixelForDirection(Vector3d(0, 1, -tv)) ) );
  EXPECT_TRUE ( VectorsEqual( Vector2d(5.5, 11), *cameraModel2.pixelForDirection(Vector3d(0, 1, tv)) ) );
}
