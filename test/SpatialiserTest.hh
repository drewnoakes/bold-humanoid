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
  auto rangeVertical = M_PI/2;
  auto rangeHorizontal = M_PI/2;

  shared_ptr<CameraModel> cameraModel = make_shared<CameraModel>(imageWidth, imageHeight, focalLength, rangeVertical, rangeHorizontal);

  return Spatialiser(cameraModel);
}

TEST (SpatialiserTests, findGroundPointForPixelLookingStraightDown)
{
  Spatialiser spatialiser = createTestSpatialiser();
  // Look straight down at the ground
  Affine3d cameraTorsoTransform(AngleAxisd(-M_PI/2, Vector3d::UnitX()));

  Maybe<Vector3d> groundPoint =
    spatialiser.findGroundPointForPixel(Vector2i(5,5), /*torsoHeight*/0.4, cameraTorsoTransform);

  EXPECT_TRUE ( VectorsEqual(cameraTorsoTransform.matrix().col(0).head<3>(), Vector3d(1, 0,  0)) );
  EXPECT_TRUE ( VectorsEqual(cameraTorsoTransform.matrix().col(1).head<3>(), Vector3d(0, 0, -1)) );
  EXPECT_TRUE ( VectorsEqual(cameraTorsoTransform.matrix().col(2).head<3>(), Vector3d(0, 1,  0)) );

  ASSERT_TRUE ( groundPoint.hasValue() );
  EXPECT_TRUE ( VectorsEqual(Vector3d(0,0,-0.4), *groundPoint.value()) );
}

TEST (SpatialiserTests, findGroundPointForPixelEmptyIfSkybound)
{
  Spatialiser spatialiser = createTestSpatialiser();

  // Look straight up at the sky
  Affine3d cameraTorsoTransform(AngleAxisd(M_PI/2, Vector3d::UnitX()));

  Maybe<Vector3d> groundPoint = spatialiser.findGroundPointForPixel(Vector2i(5,5), /*torsoHeight*/0.4, cameraTorsoTransform);

  EXPECT_FALSE ( groundPoint.hasValue() );
}

TEST (SpatialiserTests, findHorizonForColumn)
{
  Spatialiser spatialiser = createTestSpatialiser();
  // Look straight ahead
  Affine3d cameraTorsoTransform(Matrix4d::Identity());

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
  
  /*
  // Vertical displacement should not matter
  auto trans = Translation3d(0, 0, 1.0);
  cameraTorsoTransform =  cameraTorsoTransform * trans;

  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(0, cameraTorsoTransform) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(5, cameraTorsoTransform) );
  EXPECT_EQ ( 5, spatialiser.findHorizonForColumn(10, cameraTorsoTransform) );
  */
}
