#pragma once

#include "gtest/gtest.h"

#include <Eigen/Core>

::testing::AssertionResult VectorsEqual(Eigen::Vector3d const& a, Eigen::Vector3d const& b, const float delta = 0.00001) {
  double d = (a-b).norm();
  if (d < delta)
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure() << "Actual: " << b << ", expected: " << a << " d = " << d;
}

::testing::AssertionResult VectorsEqual(Eigen::Vector2d const& a, Eigen::Vector2d const& b, const float delta = 0.00001) {
  double d = (a-b).norm();
  if (d < delta)
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure() << "Actual: " << b << ", expected: " << a << " d = " << d;
}
