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

