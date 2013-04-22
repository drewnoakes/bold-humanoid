#ifndef TESTING_HELPER_HH
#define TESTING_HELPER_HH

#include "gtest/gtest.h"

#include <Eigen/Core>

::testing::AssertionResult VectorsEqual(Vector3d const& a, Vector3d const& b, const float delta = 0.00001) {
  double d = (a-b).norm();
  if (d < delta)
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure() << "Actual: " << b << ", expected: " << a << " d = " << d;
}

::testing::AssertionResult VectorsEqual(Vector2d const& a, Vector2d const& b, const float delta = 0.00001) {
  double d = (a-b).norm();
  if (d < delta)
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure() << "Actual: " << b << ", expected: " << a << " d = " << d;
}

#endif
