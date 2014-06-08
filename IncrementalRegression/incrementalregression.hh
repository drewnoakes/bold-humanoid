#pragma once

#include "../geometry/LineSegment/LineSegment2/linesegment2.hh"
#include <Eigen/Core>
#include <Eigen/LU>

namespace bold
{
  class IncrementalRegression
  {
  public:
    IncrementalRegression()
      : d_xxSum{Eigen::Matrix2f::Zero()},
      d_xySum{Eigen::Vector2f::Zero()},
      d_nPoints{0},
      d_sqErrorSum{0.0f},
      d_dirty{false}
    {}
    
    void setSqError(float sqError)
    {
      d_sqErrorSum = sqError;
    }

    Eigen::Vector2f head() const
    {
      return d_head;
    }

    float getY(float x) const
    {
      return d_beta.dot(Eigen::Vector2f{x, 1});
    }

    float fit(Eigen::Vector2f const& point)
    {
      if (d_nPoints == 1)
        return (point - head()).norm();
      else if (d_xStart == d_xEnd)
      {
        return std::abs(point.x() - d_xStart);
      }
      else
      {
        solve();
        auto sigma = sigmaAt(point.x());
        auto yPred = getY(point.x());
        auto error = std::abs(yPred - point.y());
        return sigma == 0 ? error : error / sigma;
      }
    }

    float sigmaAt(float x)
    {
      auto _x = Eigen::Vector2f{x, 1};
      return sqrt(_x.transpose() * (1.0 / d_sqErrorSum * d_xxSum).inverse() * _x);
    }

    bool isVertical() const
    {
      return d_xStart == d_xEnd;
    }

    void addPoint(Eigen::Vector2f const& point)
    {
      d_points.push_back(point);

      auto x = Eigen::Vector2f{point.x(), 1};
      d_xxSum += x * x.transpose();
      d_xySum += x * point.y();
      d_xEnd = point.x();
      ++d_nPoints;

      if (d_nPoints == 1)
      {
        d_xStart = point.x();
        d_head = point;
      }
      else if (d_nPoints == 2)
        d_head = point;
      else
      {
        d_head = Eigen::Vector2f{point.x(), getY(point.x())};
      }
      d_dirty = true;
    }
    
    void solve()
    {
      if (d_dirty && d_nPoints > 1)
      {
        d_beta = (d_xxSum / d_nPoints).inverse() * d_xySum / d_nPoints;
        d_dirty = false;
      }
    }
    
    Eigen::Vector2f getBeta() const
    {
      return d_beta;
    }

    LineSegment2f getLineSegment()
    {
      solve();
      Eigen::Vector2f p1{d_xStart, getY(d_xStart)};
      Eigen::Vector2f p2{d_xEnd, getY(d_xEnd)};
      return LineSegment2f{p1, p2};
    }

    unsigned getNPoints() const
    {
      return d_nPoints;
    }

    float determineStandardError() const
    {
      auto X = Eigen::MatrixXf(2, d_points.size());
      auto y = Eigen::VectorXf(d_points.size());
      unsigned idx = 0;
      for (auto const& point : d_points)
      {
        X.col(idx) = Eigen::Vector2f{point.x(), 1};
        y(idx) = point.y();
        ++idx;
      }
      auto res = y - X.transpose() * d_beta;
      return sqrt(res.dot(res) / (d_points.size() - 2));
    }

    void merge(IncrementalRegression const& other)
    {
      d_xxSum += other.d_xxSum;
      d_xySum += other.d_xySum;
      d_nPoints += other.d_nPoints;
      d_xStart = std::min(d_xStart, other.d_xStart);
      d_xEnd = std::max(d_xEnd, other.d_xEnd);
      solve();
    }

private:
    std::vector<Eigen::Vector2f> d_points;

    Eigen::Matrix2f d_xxSum;
    Eigen::Vector2f d_xySum;
    unsigned d_nPoints;
    Eigen::Vector2f d_beta;
    unsigned d_xStart;
    unsigned d_xEnd;
    Eigen::Vector2f d_head;
    float d_sqErrorSum;

    bool d_dirty;
  };
}
