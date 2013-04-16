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
  auto rangeVertical = M_PI/4;
  auto rangeHorizontal = M_PI/4;

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
