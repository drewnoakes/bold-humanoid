%{
#include "../geometry/LineSegment2i.hh"
#include <Eigen/Core>
%}

%template(Vector2dPtr) std::shared_ptr<Eigen::Vector2d>;
%template(Vector2iPtr) std::shared_ptr<Eigen::Vector2i>;
%template(Vector3dPtr) std::shared_ptr<Eigen::Vector3d>;

%template(Vector2dList) std::vector<Eigen::Vector2d>;
%template(Vector2iList) std::vector<Eigen::Vector2i>;
%template(Vector3dList) std::vector<Eigen::Vector3d>;

%eigen_typemaps(Eigen::Vector2d)
%eigen_typemaps(Eigen::Vector2i)
%eigen_typemaps(Eigen::Vector3d)

namespace bold
{
  struct LineSegment2i
  {
  public:
    Eigen::Vector2i p1() const;
    Eigen::Vector2i p2() const;
  };
}

namespace bold
{
  struct LineSegment3d
  {
  public:
    Eigen::Vector3d p1() const;
    Eigen::Vector3d p2() const;
  };
}
