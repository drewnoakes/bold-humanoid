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
