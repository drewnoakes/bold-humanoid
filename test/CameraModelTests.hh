#include "gtest/gtest.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "../CameraModel/cameramodel.hh"
#include "../geometry/Bounds2i.hh"
#include "../geometry/LineSegment2i.hh"

TEST (CameraModelTests, getProjector)
{
  auto imageWidth = 640;
  auto imageHeight = 480;
  auto focalLength = 1;
  auto rangeVerticalDegs = 46.0; // 46 degrees from top to bottom
  auto rangeHorizontalDegs = 60.0; // 60 degrees from left to right

  CameraModel cameraModel(imageWidth, imageHeight, focalLength, rangeVerticalDegs, rangeHorizontalDegs);

  // From 3D camera space to 2D screen space
  Affine3d projectionTransform = cameraModel.getProjectionTransform();

  EXPECT_EQ( Vector3d(0,0,0), projectionTransform.translation() ) << "No translation in projection matrix";

  std::function<Vector2i(Vector3d const&)> projector = cameraModel.getProjector();

  double projectionPlaneWidth = atan(cameraModel.rangeHorizontalRads() / 2) * cameraModel.focalLength() * 2;
  double pixelwidth = projectionPlaneWidth / cameraModel.imageWidth();

  EXPECT_EQ( Vector2i(0,0), projector(Vector3d(0,0,10)) );
  EXPECT_EQ( Vector2i(10/pixelwidth,0), projector(Vector3d(10,0,cameraModel.focalLength())) );
}

TEST (CameraModelTests, directionForPixel)
{
  auto imageWidth = 11;
  auto imageHeight = 11;
  auto focalLength = 1;
  auto rangeVerticalDegs = 90;
  auto rangeHorizontalDegs = 90;

  CameraModel cameraModel(imageWidth, imageHeight, focalLength, rangeVerticalDegs, rangeHorizontalDegs);

  EXPECT_TRUE ( Vector3d(0, 1, 0).isApprox( cameraModel.directionForPixel(Vector2i(5, 5)) ) );

  EXPECT_TRUE ( Vector3d(1, 1, 0).normalized().isApprox( cameraModel.directionForPixel(Vector2i( 0, 5)) ) );
  EXPECT_TRUE ( Vector3d(-1, 1, 0).normalized().isApprox( cameraModel.directionForPixel(Vector2i(10, 5)) ) );

  EXPECT_TRUE ( Vector3d(0, 1, -1).normalized().isApprox( cameraModel.directionForPixel(Vector2i(5,  0)) ) );
  EXPECT_TRUE ( Vector3d(0, 1,  1).normalized().isApprox( cameraModel.directionForPixel(Vector2i(5, 10)) ) );

  auto v = Vector3d( -1, 1, 0).normalized();
  v.x() *= 2.0/5.0;
  EXPECT_TRUE ( v.normalized().isApprox( cameraModel.directionForPixel(Vector2i(7,  5)) ) );
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
  rangeHorizontalDegs = 45;

  CameraModel cameraModel2(imageWidth, imageHeight, focalLength, rangeVerticalDegs, rangeHorizontalDegs);

  EXPECT_EQ ( Vector2i(5,5), cameraModel2.pixelForDirection(Vector3d(0, 1, 0)) );

  EXPECT_EQ ( Vector2i(0,5), cameraModel2.pixelForDirection(Vector3d(tan(M_PI/8), 1, 0)) );
  EXPECT_EQ ( Vector2i(10,5), cameraModel2.pixelForDirection(Vector3d(-tan(M_PI/8), 1, 0)) );

  EXPECT_EQ ( Vector2i(5,0), cameraModel2.pixelForDirection(Vector3d(0, 1, -tan(M_PI/8))) );
  EXPECT_EQ ( Vector2i(5, 10), cameraModel2.pixelForDirection(Vector3d(0, 1, tan(M_PI/8))) );
}
