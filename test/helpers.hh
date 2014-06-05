#pragma once

#include "gtest/gtest.h"

#include "../util/Maybe.hh"
#include "../geometry/LineSegment/linesegment.hh"

#include <Eigen/Core>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <iostream>

inline ::testing::AssertionResult VectorsEqual(Eigen::Vector3d const& a, Eigen::Vector3d const& b, const float delta = 0.00001) {
  double d = (a-b).norm();
  if (d < delta)
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure() << "Actual: " << b.transpose() << ", expected: " << a.transpose() << " d = " << d;
}

inline ::testing::AssertionResult VectorsEqual(Eigen::Vector2d const& a, Eigen::Vector2d const& b, const double delta = 0.00001) {
  double d = (a-b).norm();
  if (d < delta)
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure() << "Actual: " << b.transpose() << ", expected: " << a.transpose() << " d = " << d;
}

template<typename T, int N>
inline ::testing::AssertionResult VectorsEqual(Eigen::Matrix<T,N,1> const& a, Eigen::Matrix<T,N,1> const& b, const double delta = 0.00001) {
  double d = (a-b).norm();
  if (d < delta)
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure() << "Actual: " << b.transpose() << ", expected: " << a.transpose() << " d = " << d;
}

template<int N>
inline ::testing::AssertionResult VectorsEqual(Eigen::Matrix<int,N,1> const& a, Eigen::Matrix<int,N,1> const& b) {
  double d = (a-b).norm();
  if (d == 0)
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure() << "Actual: " << b.transpose() << ", expected: " << a.transpose() << " d = " << d;
}

inline ::testing::AssertionResult MatricesEqual(Eigen::MatrixXd const& a, Eigen::MatrixXd const& b, double delta = 0.000001) {
  double d = (a-b).array().abs().sum();
  if (d < delta)
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure() << "Actual: " << std::endl << b << std::endl << "Expected: " << std::endl << a << std::endl << " d = " << d;
}

template<typename T, int dim>
inline ::testing::AssertionResult LinesEqual(bold::LineSegment<T,dim> const& a, bold::LineSegment<T,dim> const& b, const double delta = 0.000001) {
  double d1 = (a.p1()-b.p1()).norm();
  double d2 = (a.p2()-b.p2()).norm();
  if (d1 <= delta && d2 <= delta)
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure() << "Actual: " << b << ", expected: " << a << " d1=" << d1 << " d2=" << d2;
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

inline bool operator==(Eigen::Vector2i const& a, Eigen::Vector2i const& b)
{
  return a.x() == b.x() && a.y() == b.y();
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
