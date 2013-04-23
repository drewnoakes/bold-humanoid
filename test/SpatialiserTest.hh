#include "gtest/gtest.h"

#include "helpers.hh"
#include "../CameraModel/cameramodel.hh"
#include "../Spatialiser/spatialiser.hh"
#include "../util/Maybe.hh"

#include <memory>
#include <Eigen/Core>
#include <Eigen/Geometry>

using namespace std;
using namespace bold;
using namespace Eigen;

Spatialiser createTestSpatialiser()
{
  auto imageWidth = 11;
  auto imageHeight = 11;
  auto focalLength = 1;
  auto rangeVertical = 90;
  auto rangeHorizontal = 90;

  shared_ptr<CameraModel> cameraModel = make_shared<CameraModel>(imageWidth, imageHeight, focalLength, rangeVertical, rangeHorizontal);

  return Spatialiser(cameraModel);
}

double torsoHeight = 0.4;

TEST (SpatialiserTests, findGroundPointForPixelLookingStraightDown)
{
  Spatialiser spatialiser = createTestSpatialiser();

  // Look straight down at the ground
  Affine3d cameraGroundTransform = Translation3d(0,0,torsoHeight) * AngleAxisd(-M_PI/2, Vector3d::UnitX());

  EXPECT_TRUE ( VectorsEqual(cameraGroundTransform.matrix().col(0).head<3>(), Vector3d(1, 0,  0)) );
  EXPECT_TRUE ( VectorsEqual(cameraGroundTransform.matrix().col(1).head<3>(), Vector3d(0, 0, -1)) );
  EXPECT_TRUE ( VectorsEqual(cameraGroundTransform.matrix().col(2).head<3>(), Vector3d(0, 1,  0)) );

  Maybe<Vector3d> groundPoint =
    spatialiser.findGroundPointForPixel(Vector2i(5,5), cameraGroundTransform);

  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0,0,-0.4), *groundPoint.value()) );
}

TEST (SpatialiserTests, findGroundPointForPixelFromCorner)
{
  auto imageWidth = 11;
  auto imageHeight = 11;
  auto focalLength = 1;
  auto rangeVertical = 60;
  auto rangeHorizontal = 90;

  shared_ptr<CameraModel> cameraModel = make_shared<CameraModel>(imageWidth, imageHeight, focalLength, rangeVertical, rangeHorizontal);

  Spatialiser spatialiser(cameraModel);

  // Look straight down at the ground
  Affine3d cameraGroundTransform = Translation3d(0,0,1) * AngleAxisd(-M_PI/4, Vector3d::UnitZ());

  for (int y = 0; y < 4; y++)
  {
    Maybe<Vector3d> groundPoint = spatialiser.findGroundPointForPixel(Vector2i(5,y), cameraGroundTransform);

    ASSERT_TRUE ( groundPoint.hasValue() ) << "No luck with y=" << y;
    EXPECT_NEAR( (*groundPoint.value()).x(), (*groundPoint.value()).y(), 0.001 );
  }

  for (int y = 0; y < 4; y++)
  {
    Maybe<Vector3d> groundPoint = spatialiser.findGroundPointForPixel(Vector2i(0,y), cameraGroundTransform);

    ASSERT_TRUE ( groundPoint.hasValue() ) << "No luck with y=" << y;
    EXPECT_NEAR( (*groundPoint.value()).y(), 0, 0.001 );
  }

  for (int y = 0; y < 4; y++)
  {
    Maybe<Vector3d> groundPoint = spatialiser.findGroundPointForPixel(Vector2i(10,y), cameraGroundTransform);

    ASSERT_TRUE ( groundPoint.hasValue() ) << "No luck with y=" << y;
    EXPECT_NEAR( (*groundPoint.value()).x(), 0, 0.001 );
  }
}

TEST (SpatialiserTests, findGroundPointForPixelLooking45Degrees)
{
  Spatialiser spatialiser = createTestSpatialiser();
  // Look 45 deg down at the ground
  Affine3d cameraTorsoTransform(AngleAxisd(-M_PI/4, Vector3d::UnitX()));

  Maybe<Vector3d> groundPoint(Vector3d(0,0,0));

  groundPoint =
    spatialiser.findGroundPointForPixel(Vector2i(5,5), /*torsoHeight*/1.0, cameraTorsoTransform);

  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0,1,-1), *groundPoint.value()) );

  groundPoint =
    spatialiser.findGroundPointForPixel(Vector2i(5,0), /*torsoHeight*/1.0, cameraTorsoTransform);

  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0,0,-1), *groundPoint.value()) );
}

TEST (SpatialiserTests, findGroundPointForPixelEmptyIfSkybound)
{
  Spatialiser spatialiser = createTestSpatialiser();

  // Look straight up at the sky
  Affine3d cameraGroundTransform = Translation3d(0,0,torsoHeight) * AngleAxisd(M_PI/2, Vector3d::UnitX());

  Maybe<Vector3d> groundPoint = spatialiser.findGroundPointForPixel(Vector2i(5,5), cameraGroundTransform);

  EXPECT_FALSE ( groundPoint.hasValue() );
}

TEST (SpatialiserTests, findHorizonForColumnSquareCam)
{
  Spatialiser spatialiser = createTestSpatialiser();

  // Look straight ahead
  Affine3d cameraTorsoTransform(Matrix4d::Identity());

  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(0, cameraTorsoTransform) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(5, cameraTorsoTransform) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(10, cameraTorsoTransform) );

  // Vertical displacement should not matter
  cameraTorsoTransform = Translation3d(0, 0, 1.0);

  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(0, cameraTorsoTransform) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(5, cameraTorsoTransform) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(10, cameraTorsoTransform) );

  // Look 45 degrees down
  cameraTorsoTransform =  AngleAxisd(-M_PI/4.0, Vector3d::UnitX());

  EXPECT_EQ ( 10, spatialiser.findHorizonForColumn(0, cameraTorsoTransform) );
  EXPECT_EQ ( 10, spatialiser.findHorizonForColumn(5, cameraTorsoTransform) );
  EXPECT_EQ ( 10, spatialiser.findHorizonForColumn(10, cameraTorsoTransform) );

  // Tilted 45 degrees
  cameraTorsoTransform =  AngleAxisd(-M_PI/4.0, Vector3d::UnitY());
  EXPECT_EQ ( 0, spatialiser.findHorizonForColumn(0, cameraTorsoTransform) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(5, cameraTorsoTransform) );
  EXPECT_EQ ( 10, spatialiser.findHorizonForColumn(10, cameraTorsoTransform) );

  // Rotation around z-axis should not matter
  cameraTorsoTransform = AngleAxisd(M_PI/2.0, Vector3d::UnitZ());
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(0, cameraTorsoTransform) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(5, cameraTorsoTransform) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(10, cameraTorsoTransform) );

  // Rotation around z-axis while leaning forward
  cameraTorsoTransform = AngleAxisd(-M_PI/4.0, Vector3d::UnitX()) * AngleAxisd(M_PI/2.0, Vector3d::UnitZ());
  EXPECT_EQ ( 10, spatialiser.findHorizonForColumn(0, cameraTorsoTransform) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(5, cameraTorsoTransform) );
  EXPECT_EQ ( 0, spatialiser.findHorizonForColumn(10, cameraTorsoTransform) );
}

TEST (SpatialiserTests, findHorizonForColumnWideCam)
{
  auto imageWidth = 11;
  auto imageHeight = 11;
  auto focalLength = 1;
  auto rangeVertical = 45;
  auto rangeHorizontal = 60;

  shared_ptr<CameraModel> cameraModel = make_shared<CameraModel>(imageWidth, imageHeight, focalLength, rangeVertical, rangeHorizontal);

  Spatialiser spatialiser(cameraModel);

  // Look straight ahead
  Affine3d cameraTorsoTransform(Matrix4d::Identity());

  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(0, cameraTorsoTransform) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(5, cameraTorsoTransform) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(10, cameraTorsoTransform) );

  // Vertical displacement should not matter
  cameraTorsoTransform = Translation3d(0, 0, 1.0);

  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(0, cameraTorsoTransform) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(5, cameraTorsoTransform) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(10, cameraTorsoTransform) );

  // Look 22.5 degrees down
  cameraTorsoTransform =  AngleAxisd(-M_PI/8.0, Vector3d::UnitX());

  EXPECT_EQ ( 10, spatialiser.findHorizonForColumn(0, cameraTorsoTransform) );
  EXPECT_EQ ( 10, spatialiser.findHorizonForColumn(5, cameraTorsoTransform) );
  EXPECT_EQ ( 10, spatialiser.findHorizonForColumn(10, cameraTorsoTransform) );

  // Tilt to have horizon go through corners
  // atan(2/3)
  cameraTorsoTransform =  AngleAxisd(-0.622, Vector3d::UnitY());
  EXPECT_EQ ( 0, spatialiser.findHorizonForColumn(0, cameraTorsoTransform) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(5, cameraTorsoTransform) );
  EXPECT_EQ ( 10, spatialiser.findHorizonForColumn(10, cameraTorsoTransform) );
}
