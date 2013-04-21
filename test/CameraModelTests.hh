#include "gtest/gtest.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "../CameraModel/cameramodel.hh"
#include "../geometry/Bounds2i.hh"
#include "../geometry/LineSegment2i.hh"
#include "helpers.hh"

TEST (CameraModelTests, directionForPixel)
{
  auto imageWidth = 11;
  auto imageHeight = 11;
  auto focalLength = 1;
  auto rangeVerticalDegs = 90;
  auto rangeHorizontalDegs = 90;

  CameraModel cameraModel1(imageWidth, imageHeight,
                           focalLength,
                           rangeVerticalDegs, rangeHorizontalDegs);
  
  EXPECT_TRUE ( VectorsEqual(Vector3d(0, 1, 0).normalized(),
                             cameraModel1.directionForPixel(Vector2i(5, 5)) ) );
  
  EXPECT_TRUE ( VectorsEqual(Vector3d(1, 1, 0).normalized(),
                             cameraModel1.directionForPixel(Vector2i( 0, 5)) ) );
  EXPECT_TRUE ( VectorsEqual(Vector3d(-1, 1, 0).normalized(),
                             cameraModel1.directionForPixel(Vector2i(10, 5)) ) );

  EXPECT_TRUE ( VectorsEqual(Vector3d(0, 1, -1).normalized(),
                             cameraModel1.directionForPixel(Vector2i(5,  0)) ) );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0, 1,  1).normalized(),
                             cameraModel1.directionForPixel(Vector2i(5, 10)) ) );

  auto v = Vector3d( -1, 1, 0).normalized();
  v.x() *= 2.0/5.0;
  EXPECT_TRUE ( VectorsEqual(v.normalized(),
                             cameraModel1.directionForPixel(Vector2i(7,  5)) ) );

  rangeVerticalDegs = 45;
  rangeHorizontalDegs = 60;

  double th = tan(.5 * 60.0 / 180.0 * M_PI);
  double tv = tan(.5 * 45.0 / 180.0 * M_PI);

  CameraModel cameraModel2(imageWidth, imageHeight,
                           focalLength,
                           rangeVerticalDegs, rangeHorizontalDegs);
  
  EXPECT_TRUE ( VectorsEqual(Vector3d(0, 1, 0).normalized(),
                             cameraModel2.directionForPixel(Vector2i(5, 5)) ) );
  
  EXPECT_TRUE ( VectorsEqual(Vector3d(th, 1, 0).normalized(),
                             cameraModel2.directionForPixel(Vector2i( 0, 5)) ) );
  EXPECT_TRUE ( VectorsEqual(Vector3d(-th, 1, 0).normalized(),
                             cameraModel2.directionForPixel(Vector2i(10, 5)) ) );

  EXPECT_TRUE ( VectorsEqual(Vector3d(0, 1, -tv).normalized(),
                             cameraModel2.directionForPixel(Vector2i(5,  0)) ) );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0, 1,  tv).normalized(),
                             cameraModel2.directionForPixel(Vector2i(5, 10)) ) );

}

TEST (CameraModelTests, pixelForDirection)
{
  auto imageWidth = 11;
  auto imageHeight = 11;
  auto focalLength = 1;
  auto rangeVerticalDegs = 90;
  auto rangeHorizontalDegs = 90;

  CameraModel cameraModel1(imageWidth, imageHeight, focalLength, rangeVerticalDegs, rangeHorizontalDegs);

  EXPECT_EQ ( Vector2i(5,5), cameraModel1.pixelForDirection(Vector3d(0, 1, 0)) );

  EXPECT_EQ ( Vector2i(0,5), cameraModel1.pixelForDirection(Vector3d(1, 1, 0)) );
  EXPECT_EQ ( Vector2i(10,5), cameraModel1.pixelForDirection(Vector3d(-1, 1, 0)) );

  EXPECT_EQ ( Vector2i(5,0), cameraModel1.pixelForDirection(Vector3d(0, 1, -1)) );
  EXPECT_EQ ( Vector2i(5, 10), cameraModel1.pixelForDirection(Vector3d(0, 1, 1)) );

  auto v = Vector3d( -1, 1, 0).normalized();
  v.x() *= 2.0/5.0;
  EXPECT_EQ ( Vector2i(7, 5), cameraModel1.pixelForDirection(v) );

  // 2 time range
  rangeVerticalDegs = 45;
  rangeHorizontalDegs = 60;

  CameraModel cameraModel2(imageWidth, imageHeight, focalLength, rangeVerticalDegs, rangeHorizontalDegs);

  double th = tan(.5 * 60.0 / 180.0 * M_PI);
  double tv = tan(.5 * 45.0 / 180.0 * M_PI);

  EXPECT_EQ ( Vector2i(5,5), cameraModel2.pixelForDirection(Vector3d(0, 1, 0)) );

  EXPECT_EQ ( Vector2i(0,5), cameraModel2.pixelForDirection(Vector3d(th, 1, 0)) );
  EXPECT_EQ ( Vector2i(10,5), cameraModel2.pixelForDirection(Vector3d(-th, 1, 0)) );

  EXPECT_EQ ( Vector2i(5,0), cameraModel2.pixelForDirection(Vector3d(0, 1, -tv)) );
  EXPECT_EQ ( Vector2i(5, 10), cameraModel2.pixelForDirection(Vector3d(0, 1, tv)) );
}
