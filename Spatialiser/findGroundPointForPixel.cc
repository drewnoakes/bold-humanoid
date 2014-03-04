#include "spatialiser.ih"

Maybe<Vector3d> Spatialiser::findGroundPointForPixel(Vector2d const& pixel, double const groundZ) const
{
  auto body = State::get<BodyState>(StateTime::CameraImage);

  Affine3d const& agentCameraTr = body->getCameraAgentTransform();

  return findGroundPointForPixel(pixel, agentCameraTr, groundZ);
}

Maybe<Vector3d> Spatialiser::findGroundPointForPixel(Vector2d const& pixel,
                                                     Affine3d const& agentCameraTr,
                                                     double const groundZ) const
{
  // point in agent frame p to pixel i: i = T * p =
 
  // | t11 t12 t13 t14 |   | p1 |   | u * w |
  // | t21 t22 t23 t24 |   | p2 |   | v * w |
  // | t31 t32 t33 t34 | * | p3 | = | w     |
  // | t41 t42 t43 t44 |   | 1  |   | 1     |
  
  // We have fixed p3, and t41 and t42 are always 0, so
  // simplification:
  
  // | t11 t12 t14 |   | p1 |   | u * w - t13 * p3 |
  // | t21 t22 t24 |   | p2 |   | v * w - t23 * p3 |
  // | t31 t32 t34 | * | 1  | = | w     - t33 * p3 |

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
  
  auto TpInv = Tp.inverse();

  auto b = Vector3d{pixel(0) - d_cameraModel->imageWidth() / 2.0, pixel(1) - d_cameraModel->imageHeight() / 2.0, 1};

  auto pt = TpInv * b;

  if (pt(2) < 0)
    return Maybe<Vector3d>::empty();
  else
  {
    auto point = Vector3d{pt(0) / pt(2), pt(1) / pt(2), 0.0};
    return Maybe<Vector3d>{point};
  }
}

Maybe<Vector2d> Spatialiser::findPixelForAgentPoint(Vector3d const& agentPoint) const
{
  return findPixelForAgentPoint(agentPoint, State::get<BodyState>(StateTime::CameraImage)->getAgentCameraTransform());
}

Maybe<Vector2d> Spatialiser::findPixelForAgentPoint(Vector3d const& agentPoint, Affine3d const& agentCameraTransform) const
{
  return d_cameraModel->pixelForDirection(agentCameraTransform * agentPoint);
}
