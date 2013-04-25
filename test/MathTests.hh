#include "gtest/gtest.h"

#include "../Math/math.hh"
#include "helpers.hh"

#include <Eigen/Core>

using namespace Eigen;
using namespace bold;

TEST (MathTests, intersectRayWithPlane)
{
  // Intersect ray pointing straight downwards
  EXPECT_EQ ( Maybe<Vector3d>(Vector3d(1,2,0)), Math::intersectRayWithPlane(Vector3d(1,2,3), Vector3d(0,0,-1), Vector4d(0,0,1,0)) );
  // As above, but flip plane (intersect from other side)
  EXPECT_EQ ( Maybe<Vector3d>(Vector3d(1,2,0)), Math::intersectRayWithPlane(Vector3d(1,2,3), Vector3d(0,0,-1), Vector4d(0,0,-1,0)) );

  // Ray's position is on plane
  EXPECT_EQ ( Maybe<Vector3d>(Vector3d(1,2,0)), Math::intersectRayWithPlane(Vector3d(1,2,0), Vector3d(0,0,1), Vector4d(0,0,1,0)) );

  // Intersect x-axis with plane perpendicular to x-axis
  EXPECT_EQ ( Maybe<Vector3d>(Vector3d(1,0,0)),
              Math::intersectRayWithPlane(Vector3d(0,0,0), Vector3d(1,0,0), Vector4d(-1,0,0,1)) );

  // Parallel ray doesn't intersect
  EXPECT_EQ ( Maybe<Vector3d>::empty(),
              Math::intersectRayWithPlane(Vector3d(0,0,1), Vector3d(1,0,0), Vector4d(0,0,1,0)) );

  // Coplanar ray doesn't intersect
  EXPECT_EQ ( Maybe<Vector3d>::empty(),
              Math::intersectRayWithPlane(Vector3d(0,0,0), Vector3d(1,0,0), Vector4d(0,0,1,0)) );

  // Ray that points away from plane doesn't intersect
  EXPECT_EQ ( Maybe<Vector3d>::empty(),
              Math::intersectRayWithPlane(Vector3d(0,0,1), Vector3d(0,0,1), Vector4d(0,0,1,0)) );
}

TEST (MathTests, intersectRayWithGroundPlane)
{
  // Intersect ray pointing straight downwards
  EXPECT_EQ ( Maybe<Vector3d>(Vector3d(1,2,0)), Math::intersectRayWithGroundPlane(Vector3d(1,2,3), Vector3d(0,0,-1), 0) );

  // Ray's position is on plane
  EXPECT_EQ ( Maybe<Vector3d>(Vector3d(1,2,0)), Math::intersectRayWithGroundPlane(Vector3d(1,2,0), Vector3d(0,0,1), 0) );

  // Parallel ray doesn't intersect
  EXPECT_EQ ( Maybe<Vector3d>::empty(),
              Math::intersectRayWithGroundPlane(Vector3d(0,0,1), Vector3d(1,0,0), 0) );

  // Coplanar ray doesn't intersect
  EXPECT_EQ ( Maybe<Vector3d>::empty(),
              Math::intersectRayWithGroundPlane(Vector3d(0,0,0), Vector3d(1,0,0), 0) );

  // Ray that points away from plane doesn't intersect
  EXPECT_EQ ( Maybe<Vector3d>::empty(),
              Math::intersectRayWithGroundPlane(Vector3d(0,0,1), Vector3d(0,0,1), 0) );

  // Ground plane not at zero
  EXPECT_EQ ( Maybe<Vector3d>(Vector3d(1,2,1)),
              Math::intersectRayWithGroundPlane(Vector3d(1,2,3), Vector3d(0,0,-1), 1) );

  // Diagonal ray
  EXPECT_TRUE ( VectorsEqual(
		  Vector3d(1,1,0),
		  *(Math::intersectRayWithGroundPlane(Vector3d(0,0,1),
						      Vector3d(1,1,-1).normalized(),
						      0).value())
		  ) );
  EXPECT_TRUE ( VectorsEqual(
		  Vector3d(2,2,0),
		  *(Math::intersectRayWithGroundPlane(Vector3d(0,0,2),
						      Vector3d(1,1,-1).normalized(),
						      0).value())
		  ) );
}

TEST (MathTests, findPerpendicularVector)
{
  EXPECT_TRUE( VectorsEqual (Vector2d( 0,-1), Math::findPerpendicularVector(Vector2d( 1, 0))) );
  EXPECT_TRUE( VectorsEqual (Vector2d( 0, 1), Math::findPerpendicularVector(Vector2d(-1, 0))) );
  EXPECT_TRUE( VectorsEqual (Vector2d( 1, 0), Math::findPerpendicularVector(Vector2d( 0, 1))) );
  EXPECT_TRUE( VectorsEqual (Vector2d(-1, 0), Math::findPerpendicularVector(Vector2d( 0,-1))) );
}

TEST (MathTests, linePointClosestToPoint)
{
  LineSegment2d unitX(Vector2d(0,0), Vector2d::UnitX());

  EXPECT_TRUE( VectorsEqual (Vector2d(  0, 0), Math::linePointClosestToPoint(unitX, Vector2d(  0, 0))) ) << "At p1";
  EXPECT_TRUE( VectorsEqual (Vector2d(  1, 0), Math::linePointClosestToPoint(unitX, Vector2d(  1, 0))) ) << "At p2";
  EXPECT_TRUE( VectorsEqual (Vector2d(  0, 0), Math::linePointClosestToPoint(unitX, Vector2d(  0, 1))) ) << "Above p1";
  EXPECT_TRUE( VectorsEqual (Vector2d(  0, 0), Math::linePointClosestToPoint(unitX, Vector2d(  0,-1))) ) << "Below p1";
  EXPECT_TRUE( VectorsEqual (Vector2d(0.5, 0), Math::linePointClosestToPoint(unitX, Vector2d(0.5, 1))) ) << "Above midpoint";
  EXPECT_TRUE( VectorsEqual (Vector2d(0.5, 0), Math::linePointClosestToPoint(unitX, Vector2d(0.5,-1))) ) << "Below midpoint";
  EXPECT_TRUE( VectorsEqual (Vector2d(  0, 0), Math::linePointClosestToPoint(unitX, Vector2d( -1, 0))) ) << "Beyond p1";
  EXPECT_TRUE( VectorsEqual (Vector2d(  1, 0), Math::linePointClosestToPoint(unitX, Vector2d(  2, 0))) ) << "Beyond p2";
}

TEST (MathTests, createNormalRng)
{
  double mean = 1.5;
  auto rng = Math::createNormalRng(mean, 1);
  double sum = 0;
  unsigned loopCount = 1000;
  for (int i = 0; i < loopCount; i++)
  {
    sum += rng();
  }
  EXPECT_NEAR(mean, sum/loopCount, 0.1);
}

TEST (MathTests, degreesRadiansConversion)
{
  EXPECT_EQ( 0, Math::degToRad(0) );
  EXPECT_EQ( 0, Math::radToDeg(0) );

  EXPECT_EQ( M_PI, Math::degToRad(180) );
  EXPECT_EQ( 180.0, Math::radToDeg(M_PI) );

  EXPECT_EQ( M_PI/2, Math::degToRad(90) );
  EXPECT_EQ( 90.0, Math::radToDeg(M_PI/2) );

  EXPECT_EQ( M_PI/4, Math::degToRad(45) );
  EXPECT_EQ( 45.0, Math::radToDeg(M_PI/4) );

  EXPECT_EQ( -M_PI/4, Math::degToRad(-45) );
  EXPECT_EQ( -45.0, Math::radToDeg(-M_PI/4) );
}

TEST (MathTests, smallestAngleBetween)
{
  EXPECT_NEAR( 0, Math::smallestAngleBetween(Vector2d(0,1), Vector2d(0,1)), 0.0000001 );
  EXPECT_NEAR( 0, Math::smallestAngleBetween(Vector2d(1,1), Vector2d(1,1)), 0.0000001 );
  EXPECT_NEAR( 0, Math::smallestAngleBetween(Vector2d(-1,-1), Vector2d(-1,-1)), 0.0000001 );

  EXPECT_NEAR( M_PI, Math::smallestAngleBetween(Vector2d(0,1), Vector2d(0,-1)), 0.0000001 );
  EXPECT_NEAR( M_PI, Math::smallestAngleBetween(Vector2d(1,1), Vector2d(-1,-1)), 0.0000001 );
  EXPECT_NEAR( M_PI, Math::smallestAngleBetween(Vector2d(1,0), Vector2d(-1,0)), 0.0000001 );

  EXPECT_NEAR( M_PI/2, Math::smallestAngleBetween(Vector2d(0,1), Vector2d(1,0)), 0.0000001 );
  EXPECT_NEAR( M_PI/2, Math::smallestAngleBetween(Vector2d(0,1), Vector2d(-1,0)), 0.0000001 );

  EXPECT_NEAR( M_PI/4, Math::smallestAngleBetween(Vector2d(0,1), Vector2d(1,1)), 0.0000001 );
  EXPECT_NEAR( M_PI/4, Math::smallestAngleBetween(Vector2d(0,1), Vector2d(-1,1)), 0.0000001 );
  EXPECT_NEAR( M_PI/4, Math::smallestAngleBetween(Vector2d(-1,-1), Vector2d(-1,0)), 0.0000001 );
}
