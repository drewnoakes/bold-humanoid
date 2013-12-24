#pragma once

#include "gtest/gtest.h"
#include "../util/Maybe.hh"

#include <Eigen/Core>

::testing::AssertionResult VectorsEqual(Eigen::Vector3d const& a, Eigen::Vector3d const& b, const float delta = 0.00001) {
  double d = (a-b).norm();
  if (d < delta)
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure() << "Actual: " << b.transpose() << ", expected: " << a.transpose() << " d = " << d;
}

::testing::AssertionResult VectorsEqual(Eigen::Vector2d const& a, Eigen::Vector2d const& b, const float delta = 0.00001) {
  double d = (a-b).norm();
  if (d < delta)
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure() << "Actual: " << b.transpose() << ", expected: " << a.transpose() << " d = " << d;
}

::testing::AssertionResult VectorsEqual(Eigen::Vector2i const& a, Eigen::Vector2i const& b) {
  double d = (a-b).norm();
  if (d == 0)
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure() << "Actual: " << b.transpose() << ", expected: " << a.transpose() << " d = " << d;
}

#define ASSERT_EMPTY(condition) \
  GTEST_TEST_BOOLEAN_(!(condition.hasValue()), #condition, Non-empty, Empty, \
                      GTEST_FATAL_FAILURE_) << *condition

#define EXPECT_EMPTY(condition) \
  GTEST_TEST_BOOLEAN_(!(condition.hasValue()), #condition, Non-empty, Empty, \
                      GTEST_NONFATAL_FAILURE_) << *condition
