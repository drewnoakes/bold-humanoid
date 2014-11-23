#include <gtest/gtest.h>

#include "../Math/math.hh"
#include "helpers.hh"
#include "../geometry/LineSegment/LineSegment2/linesegment2.hh"

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
						      0))
		  ) );
  EXPECT_TRUE ( VectorsEqual(
		  Vector3d(2,2,0),
		  *(Math::intersectRayWithGroundPlane(Vector3d(0,0,2),
						      Vector3d(1,1,-1).normalized(),
						      0))
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
  auto loopCount = unsigned{1000};
  for (auto i = unsigned{0}; i < loopCount; i++)
  {
    sum += rng();
  }
  EXPECT_NEAR(mean, sum/loopCount, 0.2);
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

TEST (MathTests, alignUp)
{
  auto transform = Affine3d::Identity();

  EXPECT_TRUE( MatricesEqual( transform.matrix(), Math::alignUp(transform).matrix() ) );

  auto rotated = AngleAxisd(.5 * M_PI, Vector3d(0, 0, 1)) * transform;
  EXPECT_TRUE( MatricesEqual( rotated.matrix(), Math::alignUp(rotated).matrix() ) );
  rotated = AngleAxisd(M_PI, Vector3d(0, 0, 1)) * transform;
  EXPECT_TRUE( MatricesEqual( rotated.matrix(), Math::alignUp(rotated).matrix() ) );
  rotated = AngleAxisd(1.5 * M_PI, Vector3d(0, 0, 1)) * transform;
  EXPECT_TRUE( MatricesEqual( rotated.matrix(), Math::alignUp(rotated).matrix() ) );
  rotated = AngleAxisd(2 * M_PI, Vector3d(0, 0, 1)) * transform;
  EXPECT_TRUE( MatricesEqual( rotated.matrix(), Math::alignUp(rotated).matrix() ) );
  rotated = AngleAxisd(0.64637 * M_PI, Vector3d(0, 0, 1)) * transform;
  EXPECT_TRUE( MatricesEqual( rotated.matrix(), Math::alignUp(rotated).matrix() ) );

  rotated = AngleAxisd(0.1 * M_PI, Vector3d(1, 0, 0)) * transform;
  EXPECT_TRUE( MatricesEqual( transform.matrix(), Math::alignUp(rotated).matrix() ) );
  rotated = AngleAxisd(0.25 * M_PI, Vector3d(1, 0, 0)) * transform;
  EXPECT_TRUE( MatricesEqual( transform.matrix(), Math::alignUp(rotated).matrix() ) );
  rotated = AngleAxisd(0.49 * M_PI, Vector3d(1, 0, 0)) * transform;
  EXPECT_TRUE( MatricesEqual( transform.matrix(), Math::alignUp(rotated).matrix() ) );

  rotated = AngleAxisd(-0.1 * M_PI, Vector3d(1, 0, 0)) * transform;
  EXPECT_TRUE( MatricesEqual( transform.matrix(), Math::alignUp(rotated).matrix() ) );
  rotated = AngleAxisd(-0.25 * M_PI, Vector3d(1, 0, 0)) * transform;
  EXPECT_TRUE( MatricesEqual( transform.matrix(), Math::alignUp(rotated).matrix() ) );
  rotated = AngleAxisd(-0.49 * M_PI, Vector3d(1, 0, 0)) * transform;
  EXPECT_TRUE( MatricesEqual( transform.matrix(), Math::alignUp(rotated).matrix() ) );

  rotated = AngleAxisd(0.1 * M_PI, Vector3d(0, 1, 0)) * transform;
  EXPECT_TRUE( MatricesEqual( transform.matrix(), Math::alignUp(rotated).matrix() ) );
  rotated = AngleAxisd(0.25 * M_PI, Vector3d(0, 1, 0)) * transform;
  EXPECT_TRUE( MatricesEqual( transform.matrix(), Math::alignUp(rotated).matrix() ) );
  rotated = AngleAxisd(0.49 * M_PI, Vector3d(0, 1, 0)) * transform;
  EXPECT_TRUE( MatricesEqual( transform.matrix(), Math::alignUp(rotated).matrix() ) );

  rotated = AngleAxisd(-0.1 * M_PI, Vector3d(0, 1, 0)) * transform;
  EXPECT_TRUE( MatricesEqual( transform.matrix(), Math::alignUp(rotated).matrix() ) );
  rotated = AngleAxisd(-0.25 * M_PI, Vector3d(0, 1, 0)) * transform;
  EXPECT_TRUE( MatricesEqual( transform.matrix(), Math::alignUp(rotated).matrix() ) );
  rotated = AngleAxisd(-0.49 * M_PI, Vector3d(0, 1, 0)) * transform;
  EXPECT_TRUE( MatricesEqual( transform.matrix(), Math::alignUp(rotated).matrix() ) );

  rotated = AngleAxisd(0.1 * M_PI, Vector3d(1, 1, 0).normalized()) * transform;
  EXPECT_TRUE( MatricesEqual( transform.matrix(), Math::alignUp(rotated).matrix() ) );
  rotated = AngleAxisd(0.25 * M_PI, Vector3d(1, 1, 0).normalized()) * transform;
  EXPECT_TRUE( MatricesEqual( transform.matrix(), Math::alignUp(rotated).matrix() ) );
  rotated = AngleAxisd(0.49 * M_PI, Vector3d(1, 1, 0).normalized()) * transform;
  EXPECT_TRUE( MatricesEqual( transform.matrix(), Math::alignUp(rotated).matrix() ) );

}

TEST(MathTests, lerp)
{
  // Ratio based
  EXPECT_NEAR( 0, Math::lerp(0, 0, 1), 0.00001 );
  EXPECT_NEAR( 0, Math::lerp(0, 0, 0), 0.00001 );
  EXPECT_NEAR( 0, Math::lerp(0, 0, 100), 0.00001 );
  EXPECT_NEAR( 5, Math::lerp(0, 5, 100), 0.00001 );

  EXPECT_NEAR( 5, Math::lerp(0.5, 0, 10), 0.00001 );
  EXPECT_NEAR( 5, Math::lerp(0.5, 10, 0), 0.00001 );

  EXPECT_NEAR( 5, Math::lerp(1, 4, 5), 0.00001 );
  EXPECT_NEAR( 20, Math::lerp(2, 0, 10), 0.00001 );
  EXPECT_NEAR( -10, Math::lerp(-1, 0, 10), 0.00001 );

  // Map based
  EXPECT_NEAR( 10, Math::lerp(0, 0, 1, 10, 20), 0.00001 );
  EXPECT_NEAR( 15, Math::lerp(0.5, 0, 1, 10, 20), 0.00001 );
  EXPECT_NEAR( 20, Math::lerp(1, 0, 1, 10, 20), 0.00001 );
  // outside range
  // TODO this is inconsistent with the other lerp function which extends beyond the given domain
  EXPECT_NEAR( 20, Math::lerp(2, 0, 1, 10, 20), 0.00001 ); // other lerp gives 30
  EXPECT_NEAR( 10, Math::lerp(0, 1, 2, 10, 20), 0.00001 ); // other lerp gives 0

  EXPECT_TRUE ( VectorsEqual(Vector2d(0,0), Math::lerp(0.0, Vector2d(0,0), Vector2d(10,10))) );
  EXPECT_TRUE ( VectorsEqual(Vector2d(5,5), Math::lerp(0.5, Vector2d(0,0), Vector2d(10,10))) );
  EXPECT_TRUE ( VectorsEqual(Vector2d(9,9), Math::lerp(0.9, Vector2d(0,0), Vector2d(10,10))) );
}

Quaterniond create(double pitch, double roll, double yaw)
{
  AngleAxisd pitchAngle(pitch, Vector3d::UnitX());
  AngleAxisd rollAngle(roll, Vector3d::UnitY());
  AngleAxisd yawAngle(yaw, Vector3d::UnitZ());

  Quaternion<double> q = rollAngle * pitchAngle * yawAngle;
  return q;
}

TEST(MathTests, normaliseRads)
{
  // Ensures return value is in range [-PI,PI)
  EXPECT_NEAR ( 0.0, Math::normaliseRads(0), 0.0001 );
  EXPECT_NEAR ( -M_PI, Math::normaliseRads(M_PI), 0.0001 );
  EXPECT_NEAR ( -M_PI, Math::normaliseRads(-M_PI), 0.0001 );
  EXPECT_NEAR ( M_PI/2, Math::normaliseRads(M_PI/2), 0.0001 );
  EXPECT_NEAR ( -M_PI/2, Math::normaliseRads(-M_PI/2), 0.0001 );
  EXPECT_NEAR ( 0, Math::normaliseRads(6*M_PI), 0.0001 );
}

TEST(MathTests, angleDiffRads)
{
  EXPECT_NEAR ( 0.0, Math::angleDiffRads(M_PI, M_PI), 0.0001 );
  EXPECT_NEAR ( 0.25 * M_PI, Math::angleDiffRads(0, 0.25 * M_PI), 0.0001 );
  EXPECT_NEAR ( 0.25 * M_PI, Math::angleDiffRads(1.75 * M_PI, 0), 0.0001 );
  EXPECT_NEAR ( M_PI, Math::angleDiffRads(.25 * M_PI, 1.25 * M_PI), 0.0001 );
  EXPECT_NEAR ( M_PI, Math::angleDiffRads(1.25 * M_PI, 0.25 * M_PI), 0.0001 );
  EXPECT_NEAR ( 1.5 * M_PI, Math::angleDiffRads(0, 1.5 * M_PI), 0.0001 );
  EXPECT_NEAR ( 1.5 * M_PI, Math::angleDiffRads(0.5 * M_PI, 0), 0.0001 );
}

TEST(MathTests, shortestAngleDiffRads)
{
  // Assumes input angles are both in range [-PI,PI)
  EXPECT_NEAR ( 0.0, Math::shortestAngleDiffRads(0, 0), 0.0001 );
  EXPECT_NEAR ( 0.1, Math::shortestAngleDiffRads(0, 0.1), 0.0001 );
  EXPECT_NEAR ( -0.1, Math::shortestAngleDiffRads(0.1, 0), 0.0001 );
  EXPECT_NEAR ( 0.0, Math::shortestAngleDiffRads(0, 2*M_PI), 0.0001 );
  EXPECT_NEAR ( 0.0, Math::shortestAngleDiffRads(-M_PI, M_PI), 0.0001 );
  EXPECT_NEAR ( M_PI, Math::shortestAngleDiffRads(-M_PI/2, M_PI/2), 0.0001 );
  EXPECT_NEAR ( -M_PI/2, Math::shortestAngleDiffRads(0, 3*M_PI/2), 0.0001 );
  EXPECT_NEAR ( M_PI/2, Math::shortestAngleDiffRads(0, -3*M_PI/2), 0.0001 );
  EXPECT_NEAR ( 2*M_PI/3, Math::shortestAngleDiffRads(-M_PI/3, M_PI/3), 0.0001 );
  EXPECT_NEAR ( -2*M_PI/3, Math::shortestAngleDiffRads(-2*M_PI/3, 2*M_PI/3), 0.0001 );
  EXPECT_NEAR ( 2*M_PI/3, Math::shortestAngleDiffRads(2*M_PI/3, -2*M_PI/3), 0.0001 );
  EXPECT_NEAR ( -M_PI/3, Math::shortestAngleDiffRads(0, 11*M_PI/3), 0.0001 );
  EXPECT_NEAR ( 2.00605, Math::shortestAngleDiffRads(-2.44379, -0.43774), 0.0001 );
}

TEST(MathTests, angleToPoint)
{
  EXPECT_EQ ( 0, Math::angleToPoint(Vector2d(0, 2)) );
  EXPECT_EQ ( 0, Math::angleToPoint(Vector2d(0, 3)) );
  EXPECT_NEAR (  M_PI/4, Math::angleToPoint(Vector2d(-1, 1)), 0.0001 );
  EXPECT_NEAR ( -M_PI/4, Math::angleToPoint(Vector2d(1, 1)), 0.0001 );
  EXPECT_NEAR (  M_PI/2, Math::angleToPoint(Vector2d(-1, 0)), 0.0001 );
  EXPECT_NEAR ( -M_PI/2, Math::angleToPoint(Vector2d(1, 0)), 0.0001 );
  EXPECT_NEAR ( -M_PI,   Math::angleToPoint(Vector2d(0, -1)), 0.0001 );
}
