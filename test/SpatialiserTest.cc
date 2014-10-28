#include <gtest/gtest.h>

#include "helpers.hh"
#include "../CameraModel/cameramodel.hh"
#include "../Spatialiser/spatialiser.hh"
#include "../util/Maybe.hh"
#include "../util/memory.hh"

#include <memory>
#include <Eigen/Core>
#include <Eigen/Geometry>

using namespace std;
using namespace bold;
using namespace Eigen;

auto imageWidth = 11;
auto imageHeight = 11;

Spatialiser createTestSpatialiser(double rangeVertical = 90, double rangeHorizontal = 90)
{
  shared_ptr<CameraModel> cameraModel = allocate_aligned_shared<CameraModel>(imageWidth, imageHeight, rangeVertical, rangeHorizontal);
  return Spatialiser(cameraModel);
}

TEST (SpatialiserTests, findGroundPointForPixelLookingStraightDown)
{
  Spatialiser spatialiser = createTestSpatialiser();

  // Look straight down at the ground
  Affine3d agentCameraTr1 = Translation3d(0,0,1) * AngleAxisd(-M_PI/2, Vector3d::UnitX());
  Affine3d agentCameraTr2 = Translation3d(0,0,1.5) * AngleAxisd(-M_PI/2, Vector3d::UnitX());

  // Columns should give camera frame in agent frame:
  // Rotated around x, so unchanged
  // y now points down
  // z points foreward
  EXPECT_TRUE ( VectorsEqual(agentCameraTr1.matrix().col(0).head<3>(), Vector3d(1, 0,  0)) );
  EXPECT_TRUE ( VectorsEqual(agentCameraTr1.matrix().col(1).head<3>(), Vector3d(0, 0, -1)) );
  EXPECT_TRUE ( VectorsEqual(agentCameraTr1.matrix().col(2).head<3>(), Vector3d(0, 1,  0)) );

  Maybe<Vector3d> groundPoint;

  groundPoint = spatialiser.findGroundPointForPixel(Vector2d(5.5, 5.5), agentCameraTr1);
  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0,0,0), *groundPoint) );

  groundPoint = spatialiser.findGroundPointForPixel(Vector2d(5.5, 5.5), agentCameraTr2);
  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0,0,0), *groundPoint) );
  

  groundPoint =
    spatialiser.findGroundPointForPixel(Vector2d(5.5, 11), agentCameraTr1);
  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0,1.0,0), *groundPoint) );

  groundPoint =
    spatialiser.findGroundPointForPixel(Vector2d(5.5, 11), agentCameraTr2);
  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0,1.5,0), *groundPoint) );


  groundPoint =
    spatialiser.findGroundPointForPixel(Vector2d(5.5, 0), agentCameraTr1);
  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0,-1.0,0), *groundPoint) );

  groundPoint =
    spatialiser.findGroundPointForPixel(Vector2d(5.5, 0), agentCameraTr2);
  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0,-1.5,0), *groundPoint) );


  groundPoint =
    spatialiser.findGroundPointForPixel(Vector2d(11, 5.5), agentCameraTr1);
  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(-1.0,0,0), *groundPoint) );

  groundPoint =
    spatialiser.findGroundPointForPixel(Vector2d(11, 5.5), agentCameraTr2);
  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(-1.5,0,0), *groundPoint) );


  groundPoint =
    spatialiser.findGroundPointForPixel(Vector2d(0, 5.5), agentCameraTr1);
  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(1.0,0,0), *groundPoint) );

  groundPoint =
    spatialiser.findGroundPointForPixel(Vector2d(0, 5.5), agentCameraTr2);
  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(1.5,0,0), *groundPoint) );
}

TEST (SpatialiserTests, findRaisedGroundPointForPixel)
{
  Spatialiser spatialiser = createTestSpatialiser();

  // Look straight down at the ground
  Affine3d agentCameraTr = Translation3d(0,0,2) * AngleAxisd(-M_PI/2, Vector3d::UnitX());

  Maybe<Vector3d> groundPoint;

  groundPoint = spatialiser.findGroundPointForPixel(Vector2d(5.5, 5.5), agentCameraTr, 1.0);
  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0,0,0), *groundPoint) );
  
  groundPoint =
    spatialiser.findGroundPointForPixel(Vector2d(5.5, 11), agentCameraTr, 1.0);
  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0,1.0,0), *groundPoint) );

  groundPoint =
    spatialiser.findGroundPointForPixel(Vector2d(5.5, 0), agentCameraTr, 1.0);
  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0,-1.0,0), *groundPoint) );

  groundPoint =
    spatialiser.findGroundPointForPixel(Vector2d(11, 5.5), agentCameraTr, 1.0);
  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(-1.0,0,0), *groundPoint) );

  groundPoint =
    spatialiser.findGroundPointForPixel(Vector2d(0, 5.5), agentCameraTr, 1.0);
  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(1.0,0,0), *groundPoint) );

}

TEST (SpatialiserTests, findGroundPointForPixelScanUp)
{
  // When scanning up in the image when looking straight ahead, y should increase
  Spatialiser spatialiser = createTestSpatialiser();
  auto agentCameraTr = Affine3d{Translation3d{0, 0, 1}};
   
  Maybe<Vector3d> groundPoint;
   
  groundPoint = spatialiser.findGroundPointForPixel(Vector2d(5.5, 0), agentCameraTr, 0);
  ASSERT_TRUE ( groundPoint.hasValue() );
  double previousY = groundPoint->y();
  for (unsigned y = 1; y <=5; ++y)
  {
    groundPoint = spatialiser.findGroundPointForPixel(Vector2d(5.5, y), agentCameraTr, 0);
    ASSERT_TRUE ( groundPoint.hasValue() );
    ASSERT_TRUE ( groundPoint->y() > previousY ) << "For pixel: " << y << "; y: " << groundPoint->y() << "; prev: " << previousY;
    previousY = groundPoint->y();
  }
}

TEST (SpatialiserTests, findGroundPointForPixelFromCorner)
{
  // NOTE different vertical range -- camera is wider than it is tall
  Spatialiser spatialiser = createTestSpatialiser(/*rangeVertical*/60, /*rangeHorizontal*/90);

  //  ^y _ y==x
  //  |  /|
  //  | /
  //  |/
  //  *---> x

  // From one unit of elevation, look down the y==x axis (rotated around Z by -45 degrees),
  // parallel to the Z plane.
  Affine3d agentCameraTr = Translation3d(0,0,1) * AngleAxisd(-M_PI/4, Vector3d::UnitZ());

  ASSERT_TRUE ( VectorsEqual(Vector3d(sqrt(.5), sqrt(.5), 1), agentCameraTr * Vector3d(0,1,0)) );

  for (int y = 0; y < 4; y++)
  {
    Maybe<Vector3d> groundPoint = spatialiser.findGroundPointForPixel(Vector2d(5.5,y), agentCameraTr);

    ASSERT_TRUE ( groundPoint.hasValue() ) << "No luck with y=" << y;
    EXPECT_NEAR( (*groundPoint).x(), (*groundPoint).y(), 0.001 );
  }

  for (int y = 0; y < 4; y++)
  {
    Maybe<Vector3d> groundPoint = spatialiser.findGroundPointForPixel(Vector2d(0,y), agentCameraTr);

    ASSERT_TRUE ( groundPoint.hasValue() ) << "No luck with y=" << y;
    EXPECT_NEAR( (*groundPoint).y(), 0, 0.001 );
  }

  for (int y = 0; y < 4; y++)
  {
    Maybe<Vector3d> groundPoint = spatialiser.findGroundPointForPixel(Vector2d(imageWidth,y), agentCameraTr);

    ASSERT_TRUE ( groundPoint.hasValue() ) << "No luck with y=" << y;
    EXPECT_NEAR( (*groundPoint).x(), 0, 0.001 );
  }
}

TEST (SpatialiserTests, findGroundPointForPixelLooking45DegreesDown)
{
  Spatialiser spatialiser = createTestSpatialiser();

  // Look 45 deg down at the ground
  Affine3d agentCameraTr = Translation3d(0,0,1) * AngleAxisd(-M_PI/4, Vector3d::UnitX());

  EXPECT_EQ ( 1, agentCameraTr.translation().z() );

  Maybe<Vector3d> groundPoint =
    spatialiser.findGroundPointForPixel(Vector2d(5.5,5.5), agentCameraTr);

  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0,1,0), *groundPoint) );

  groundPoint =
    spatialiser.findGroundPointForPixel(Vector2d(5.5,0), agentCameraTr);

  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0,0,0), *groundPoint) );
}

TEST (SpatialiserTests, findGroundPointForPixelEmptyIfSkybound)
{
  Spatialiser spatialiser = createTestSpatialiser();

  // Look straight up at the sky
  Affine3d agentCameraTr = Translation3d(0,0,1) * AngleAxisd(M_PI/2, Vector3d::UnitX());

  Maybe<Vector3d> groundPoint = spatialiser.findGroundPointForPixel(Vector2d(5.5,5.5), agentCameraTr);

  EXPECT_EMPTY ( groundPoint );
}

TEST (SpatialiserTests, findPixelForAgentPointLooking45DegreesDown)
{
  Spatialiser spatialiser = createTestSpatialiser();

  // Look 45 deg down at the ground from one unit above the origin
  Affine3d agentCameraTr = Translation3d(0,0,1) * AngleAxisd(-M_PI/4, Vector3d::UnitX());
  Affine3d cameraAgentTr = agentCameraTr.inverse();

  // Find the pixel for the point one metre along the y-axis from the origin.
  // This should be exactly in the middle of the camera, and therefore the image.
  Maybe<Vector2d> pixel = spatialiser.findPixelForAgentPoint(Vector3d(0,1,0), cameraAgentTr);

  ASSERT_TRUE ( pixel.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector2d(5.5, 5.5), *pixel) );

  pixel = spatialiser.findPixelForAgentPoint(Vector3d(0, 0, 0), cameraAgentTr);

  ASSERT_TRUE ( pixel.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector2d(5.5, 0), *pixel) );
}

TEST (SpatialiserTests, findPixelForAgentPointLookingStraightDown)
{
  // NOTE different vertical range -- camera is wider than it is tall
  Spatialiser spatialiser = createTestSpatialiser(/*rangeVertical*/60, /*rangeHorizontal*/90);

  // Look straight down down at the ground from one unit above the origin
  Affine3d agentCameraTr = Translation3d(0,0,1) * AngleAxisd(-M_PI/2, Vector3d::UnitX());
  Affine3d cameraAgentTr = agentCameraTr.inverse();

  Maybe<Vector2d> pixel;

  // Find the pixel for the origin.
  // This should be on the camera's y-axis, and therefore the middle of the image.
  pixel = spatialiser.findPixelForAgentPoint(Vector3d(0,0,0), cameraAgentTr);
  ASSERT_TRUE ( pixel.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector2d(5.5, 5.5), *pixel) );

  // NOTE that the image is flipped in both x and y axes
  //
  // At the right edge, pixel x = 0
  //        left edge,  pixel x = 11

  double extremeX = tan(spatialiser.getCameraModel()->rangeHorizontalRads() / 2.0);

  ASSERT_NEAR( 1, extremeX, 0.0001 );

  // Find the pixel at the RIGHT of the camera's view
  pixel = spatialiser.findPixelForAgentPoint(Vector3d(extremeX,0,0), cameraAgentTr);
  ASSERT_TRUE ( pixel.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector2d(0, imageHeight/2.0), *pixel) );

  // Find the pixel at the LEFT of the camera's view
  pixel = spatialiser.findPixelForAgentPoint(Vector3d(-extremeX,0,0), cameraAgentTr);
  ASSERT_TRUE ( pixel.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector2d(imageWidth, imageHeight/2.0), *pixel) );

  // Find the pixel halfway from the origin to the RIGHT of the camera's view
  pixel = spatialiser.findPixelForAgentPoint(Vector3d(extremeX/2.0,0,0), cameraAgentTr);
  ASSERT_TRUE ( pixel.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector2d(5.5/2.0, imageHeight/2.0), *pixel) );

  double extremeY = tan(spatialiser.getCameraModel()->rangeVerticalRads() / 2.0);

  ASSERT_NEAR( 0.577350269, extremeY, 0.0001 );

  // Find the pixel at the TOP of the camera's view
  pixel = spatialiser.findPixelForAgentPoint(Vector3d(0,-extremeY,0), cameraAgentTr);
  ASSERT_TRUE ( pixel.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector2d(5.5, 0), *pixel) );

  // Find the pixel at the BOTTOM of the camera's view
  pixel = spatialiser.findPixelForAgentPoint(Vector3d(0,extremeY,0), cameraAgentTr);
  ASSERT_TRUE ( pixel.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector2d(5.5, imageHeight), *pixel) );

  // Test some locations which are outside the viewing frustum, but still mappable to the image plane
  pixel = spatialiser.findPixelForAgentPoint(Vector3d(extremeX*2,0,0), cameraAgentTr);
  ASSERT_TRUE ( pixel.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector2d(5.5 - imageWidth, 5.5), *pixel) );

  pixel = spatialiser.findPixelForAgentPoint(Vector3d(0,extremeY*2,0), cameraAgentTr);
  ASSERT_TRUE ( pixel.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector2d(5.5,5.5 + imageHeight), *pixel) );

  // Test some locations which are undefined in the camera's field of view
  EXPECT_EMPTY ( spatialiser.findPixelForAgentPoint(Vector3d(0,0,1), cameraAgentTr) );
  EXPECT_EMPTY ( spatialiser.findPixelForAgentPoint(Vector3d(0,extremeY,1), cameraAgentTr) );
  EXPECT_EMPTY ( spatialiser.findPixelForAgentPoint(Vector3d(extremeX,0,1), cameraAgentTr) );
  EXPECT_EMPTY ( spatialiser.findPixelForAgentPoint(Vector3d(0,extremeY,1.001), cameraAgentTr) );
  EXPECT_EMPTY ( spatialiser.findPixelForAgentPoint(Vector3d(extremeX,0,1.001), cameraAgentTr) );
  EXPECT_EMPTY ( spatialiser.findPixelForAgentPoint(Vector3d(0,0,2), cameraAgentTr) );
}

TEST (SpatialiserTests, findHorizonForColumnSquareCam)
{
  Spatialiser spatialiser = createTestSpatialiser();

  // Look straight ahead
  Affine3d cameraAgentTr(Matrix4d::Identity());

  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(0, cameraAgentTr) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(5, cameraAgentTr) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(10, cameraAgentTr) );

  // Vertical displacement should not matter
  cameraAgentTr = Translation3d(0, 0, -1.0);

  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(0, cameraAgentTr) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(5, cameraAgentTr) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(10, cameraAgentTr) );

  // Look 45 degrees down
  cameraAgentTr =  AngleAxisd(-M_PI/4.0, Vector3d::UnitX()).inverse();

  EXPECT_EQ ( 10, spatialiser.findHorizonForColumn(0, cameraAgentTr) );
  EXPECT_EQ ( 10, spatialiser.findHorizonForColumn(5, cameraAgentTr) );
  EXPECT_EQ ( 10, spatialiser.findHorizonForColumn(10, cameraAgentTr) );

  // Tilted 45 degrees
  cameraAgentTr =  AngleAxisd(-M_PI/4.0, Vector3d::UnitY()).inverse();
  EXPECT_EQ ( 0, spatialiser.findHorizonForColumn(0, cameraAgentTr) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(5, cameraAgentTr) );
  EXPECT_EQ ( 10, spatialiser.findHorizonForColumn(10, cameraAgentTr) );

  // Rotation around z-axis should not matter
  cameraAgentTr = AngleAxisd(M_PI/2.0, Vector3d::UnitZ()).inverse();
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(0, cameraAgentTr) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(5, cameraAgentTr) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(10, cameraAgentTr) );

  // Rotation around z-axis while leaning forward
  cameraAgentTr = (AngleAxisd(-M_PI/4.0, Vector3d::UnitX()) * AngleAxisd(M_PI/2.0, Vector3d::UnitZ())).inverse();
  EXPECT_EQ ( 10, spatialiser.findHorizonForColumn(0, cameraAgentTr) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(5, cameraAgentTr) );
  EXPECT_EQ ( 0, spatialiser.findHorizonForColumn(10, cameraAgentTr) );
}

TEST (SpatialiserTests, findHorizonForColumnWideCam)
{
  auto imageWidth = 11;
  auto imageHeight = 11;
  auto rangeVertical = 45;
  auto rangeHorizontal = 60;

  shared_ptr<CameraModel> cameraModel = allocate_aligned_shared<CameraModel>(imageWidth, imageHeight, rangeVertical, rangeHorizontal);

  Spatialiser spatialiser(cameraModel);

  // Look straight ahead
  Affine3d cameraAgentTr(Matrix4d::Identity());

  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(0, cameraAgentTr) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(5, cameraAgentTr) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(10, cameraAgentTr) );

  // Vertical displacement should not matter
  cameraAgentTr = Translation3d(0, 0, -1.0);

  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(0, cameraAgentTr) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(5, cameraAgentTr) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(10, cameraAgentTr) );

  // Look 22.5 degrees down
  cameraAgentTr =  AngleAxisd(-M_PI/8.0, Vector3d::UnitX()).inverse();

  EXPECT_EQ ( 10, spatialiser.findHorizonForColumn(0, cameraAgentTr) );
  EXPECT_EQ ( 10, spatialiser.findHorizonForColumn(5, cameraAgentTr) );
  EXPECT_EQ ( 10, spatialiser.findHorizonForColumn(10, cameraAgentTr) );

  // Tilt to have horizon go through corners
  // atan(2/3)
  cameraAgentTr =  AngleAxisd(-0.622, Vector3d::UnitY()).inverse();
  EXPECT_EQ ( 0, spatialiser.findHorizonForColumn(0, cameraAgentTr) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(5, cameraAgentTr) );
  EXPECT_EQ ( 10, spatialiser.findHorizonForColumn(10, cameraAgentTr) );
}
