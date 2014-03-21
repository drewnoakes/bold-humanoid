#include "gtest/gtest.h"

#include "helpers.hh"
#include "../CameraModel/cameramodel.hh"
#include "../Spatialiser/spatialiser.hh"
#include "../LineJunctionFinder/linejunctionfinder.hh"

using namespace bold;
using namespace std;
using namespace Eigen;

shared_ptr<Spatialiser> createTestSpatialiser(unsigned imageWidth = 11, unsigned imageHeight = 11, double rangeVertical = 90, double rangeHorizontal = 90)
{
  shared_ptr<CameraModel> cameraModel = allocate_aligned_shared<CameraModel>(imageWidth, imageHeight, rangeVertical, rangeHorizontal);
  return make_shared<Spatialiser>(cameraModel);
}

TEST (LineJunctionFinder, findJunctions)
{
  auto spatialiser = createTestSpatialiser();

  LineJunctionFinder finder{};

  // X junctions
  auto segment1 = LineSegment3d{Vector3d{-1.0, 0.0, 0.0}, Vector3d{1.0, 0.0, 0.0}};
  auto segment2 = LineSegment3d{Vector3d{0.0, -1.0, 0.0}, Vector3d{0.0, 1.0, 0.0}};

  auto junction = finder.tryFindLineJunction(segment1, segment2);

  EXPECT_TRUE ( junction.hasValue() );

  EXPECT_EQ( junction->type, LineJunction::Type::X );
  EXPECT_TRUE( VectorsEqual( junction->position, Vector2d(0.0, 0.0) ) );


  segment1 = LineSegment3d{Vector3d{-1.0, -1.0, 0.0}, Vector3d{1.0,  1.0, 0.0}};
  segment2 = LineSegment3d{Vector3d{-1.0,  1.0, 0.0}, Vector3d{1.0, -1.0, 0.0}};

  junction = finder.tryFindLineJunction(segment1, segment2);

  EXPECT_TRUE ( junction.hasValue() );

  EXPECT_EQ( junction->type, LineJunction::Type::X );
  EXPECT_TRUE( VectorsEqual( junction->position, Vector2d(0.0, 0.0) ) );


  // T junctions
  segment1 = LineSegment3d{Vector3d{-1.0, 0.0, 0.0}, Vector3d{1.0,  0.0, 0.0}};
  segment2 = LineSegment3d{Vector3d{0.0,  0.0, 0.0}, Vector3d{0.0,  1.0, 0.0}};

  junction = finder.tryFindLineJunction(segment1, segment2);

  EXPECT_TRUE ( junction.hasValue() );

  EXPECT_EQ( junction->type, LineJunction::Type::T );
  EXPECT_TRUE( VectorsEqual( junction->position, Vector2d(0.0, 0.0) ) );


  segment1 = LineSegment3d{Vector3d{-1.0, -1.0, 0.0}, Vector3d{1.0,  1.0, 0.0}};
  segment2 = LineSegment3d{Vector3d{0.0,  0.0, 0.0}, Vector3d{1.0,  -1.0, 0.0}};

  junction = finder.tryFindLineJunction(segment1, segment2);

  EXPECT_TRUE ( junction.hasValue() );

  EXPECT_EQ( junction->type, LineJunction::Type::T );
  EXPECT_TRUE( VectorsEqual( junction->position, Vector2d(0.0, 0.0) ) );


  // L junctions
  segment1 = LineSegment3d{Vector3d{0.0, 0.0, 0.0}, Vector3d{0.0,  1.0, 0.0}};
  segment2 = LineSegment3d{Vector3d{0.0,  0.0, 0.0}, Vector3d{1.0,  0, 0.0}};

  junction = finder.tryFindLineJunction(segment1, segment2);

  EXPECT_TRUE ( junction.hasValue() );

  EXPECT_EQ( junction->type, LineJunction::Type::L );
  EXPECT_TRUE( VectorsEqual( junction->position, Vector2d(0.0, 0.0) ) );


  segment1 = LineSegment3d{Vector3d{0.0, 0.0, 0.0}, Vector3d{1.0,  1.0, 0.0}};
  segment2 = LineSegment3d{Vector3d{2.0,  0.0, 0.0}, Vector3d{1.0, 1.0, 0.0}};

  junction = finder.tryFindLineJunction(segment1, segment2);

  EXPECT_TRUE ( junction.hasValue() );

  EXPECT_EQ( junction->type, LineJunction::Type::L );
  EXPECT_TRUE( VectorsEqual( junction->position, Vector2d(1.0, 1.0) ) );


  // No junction
  segment1 = LineSegment3d{Vector3d{0.0, 0.0, 0.0}, Vector3d{1.0,  0.0, 0.0}};
  segment2 = LineSegment3d{Vector3d{0.5,  0.21, 0.0}, Vector3d{0.5, 1.0, 0.0}};

  junction = finder.tryFindLineJunction(segment1, segment2, 0.2);

  EXPECT_TRUE ( !junction.hasValue() );


  segment1 = LineSegment3d{Vector3d{0.0, 0.0, 0.0}, Vector3d{0.5,  0.5, 0.0}};
  segment2 = LineSegment3d{Vector3d{2.0,  0.0, 0.0}, Vector3d{1.5, 0.5, 0.0}};

  junction = finder.tryFindLineJunction(segment1, segment2);

  EXPECT_TRUE ( !junction.hasValue() );
}
