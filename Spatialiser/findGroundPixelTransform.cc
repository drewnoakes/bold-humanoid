#include "spatialiser.ih"


Matrix3d Spatialiser::findGroundPixelTransform(double groundZ) const
{
  return findGroundPixelTransform(State::get<BodyState>(StateTime::CameraImage)->getAgentCameraTransform(), groundZ);
}

Matrix3d Spatialiser::findGroundPixelTransform(Eigen::Affine3d const& agentCameraTr, double groundZ) const
{
  // point in agent frame p to pixel i: i = T * p =
 
  // | t11 t12 t13 t14 |   | p1 |   | u * w |
  // | t21 t22 t23 t24 |   | p2 |   | v * w |
  // | t31 t32 t33 t34 | * | p3 | = | w     |
  // | t41 t42 t43 t44 |   | 1  |   | 1     |
  
  // We have fixed p3 = 0, and t41 and t42 are always 0, so
  // simplification:
  
  // | t11 t12 t14 |   | p1 |   | u * w |
  // | t21 t22 t24 |   | p2 |   | v * w |
  // | t31 t32 t34 | * | 1  | = | w     |

  // Bringing w to other side:

  // | t11 t12 t14 |   | p1 / w |   | u |
  // | t21 t22 t24 |   | p2 / w |   | v |
  // | t31 t32 t34 | * | 1  / w | = | 1 |
    
  // Note u and v are in pixels wrt center of image
  auto groundCameraTr = Translation3d(0, 0, -groundZ) * agentCameraTr;

  // from camera frame to image frame
  auto imageCameraTr = d_cameraModel->getImageCameraTransform();

  // from agent frame to camera frame
  auto cameraGroundTr = groundCameraTr.inverse();

  // from agent frame to image frame
  auto imageGroundTr = imageCameraTr * cameraGroundTr;

  auto Tm = imageGroundTr.matrix();
  auto Tp = (Matrix3d{} << 
             Tm(0,0), Tm(0,1), Tm(0,3),
             Tm(1,0), Tm(1,1), Tm(1,3),
             Tm(2,0), Tm(2,1), Tm(2,3)).finished();
  
  auto groundPixelTr = Matrix3d{Tp.inverse()};

  return groundPixelTr;
}
