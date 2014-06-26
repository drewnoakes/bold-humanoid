#pragma once

#include "gtest/gtest.h"

#include "../util/Maybe.hh"
#include "../geometry/LineSegment/linesegment.hh"

#include <Eigen/Core>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <iostream>

inline ::testing::AssertionResult VectorsEqual(Eigen::Vector3d const& expected, Eigen::Vector3d const& actual, const float delta = 0.00001) {
  double d = (expected-actual).norm();
  if (d < delta)
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure() << "Actual: " << actual.transpose() << ", expected: " << expected.transpose() << " d = " << d;
}

inline ::testing::AssertionResult VectorsEqual(Eigen::Vector2d const& expected, Eigen::Vector2d const& actual, const double delta = 0.00001) {
  double d = (expected-actual).norm();
  if (d < delta)
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure() << "Actual: " << actual.transpose() << ", expected: " << expected.transpose() << " d = " << d;
}

template<typename T, int N>
inline ::testing::AssertionResult VectorsEqual(Eigen::Matrix<T,N,1> const& expected, Eigen::Matrix<T,N,1> const& actual, const double delta = 0.00001) {
  double d = (expected-actual).norm();
  if (d < delta)
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure() << "Actual: " << actual.transpose() << ", expected: " << expected.transpose() << " d = " << d;
}

template<int N>
inline ::testing::AssertionResult VectorsEqual(Eigen::Matrix<int,N,1> const& expected, Eigen::Matrix<int,N,1> const& actual) {
  double d = (expected-actual).norm();
  if (d == 0)
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure() << "Actual: " << actual.transpose() << ", expected: " << expected.transpose() << " d = " << d;
}

inline ::testing::AssertionResult MatricesEqual(Eigen::MatrixXd const& expected, Eigen::MatrixXd const& actual, double delta = 0.000001) {
  double d = (expected-actual).array().abs().sum();
  if (d < delta)
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure() << "Actual: " << std::endl << actual << std::endl << "Expected: " << std::endl << expected << std::endl << " d = " << d;
}

template<typename T, int dim>
inline ::testing::AssertionResult LinesEqual(bold::LineSegment<T,dim> const& expected, bold::LineSegment<T,dim> const& actual, const double delta = 0.000001) {
  double d1 = (expected.p1()-actual.p1()).norm();
  double d2 = (expected.p2()-actual.p2()).norm();
  if (d1 <= delta && d2 <= delta)
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure() << "Actual: " << actual << ", expected: " << expected << " d1=" << d1 << " d2=" << d2;
}

//PrintTo(const T&, ostream*)

inline std::ostream& operator<<(std::ostream& stream, Eigen::Vector2i const& v)
{
  return stream << "(" << v.x() << ", " << v.y() << ")";
}

inline std::ostream& operator<<(std::ostream& stream, Eigen::Vector2f const& v)
{
  return stream << "(" << v.x() << ", " << v.y() << ")";
}

inline std::ostream& operator<<(std::ostream& stream, Eigen::Vector2d const& v)
{
  return stream << "(" << v.x() << ", " << v.y() << ")";
}

inline std::ostream& operator<<(std::ostream& stream, Eigen::Vector3d const& v)
{
  return stream << "(" << v.x() << ", " << v.y() << ", " << v.z() << ")";
}

inline bool operator==(Eigen::Vector2i const& expected, Eigen::Vector2i const& actual)
{
  return expected.x() == actual.x() && expected.y() == actual.y();
}


#define ASSERT_EMPTY(condition) \
  GTEST_TEST_BOOLEAN_(!(condition.hasValue()), #condition, Non-empty, Empty, \
                      GTEST_FATAL_FAILURE_) << *condition

#define EXPECT_EMPTY(condition) \
  GTEST_TEST_BOOLEAN_(!(condition.hasValue()), #condition, Non-empty, Empty, \
                      GTEST_NONFATAL_FAILURE_) << *condition

#define EXPECT_BETWEEN(lower, upper, val) \
  do { \
  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperGE, val, lower); \
  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperLE, val, upper); \
  } while (0)
