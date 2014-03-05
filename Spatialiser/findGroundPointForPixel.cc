#include "spatialiser.ih"

Maybe<Vector3d> Spatialiser::findGroundPointForPixel(Vector2d const& pixel, double groundZ) const
{
  return findGroundPointForPixel(pixel, groundZ == 0 ?
                                 d_zeroGroundPixelTr :
                                 findGroundPixelTransform(State::get<BodyState>(StateTime::CameraImage)->getAgentCameraTransform(), groundZ));
}

Maybe<Vector3d> Spatialiser::findGroundPointForPixel(Vector2d const& pixel,
                                                     Affine3d const& agentCameraTr,
                                                     double groundZ) const
{
  auto groundPixelTr = findGroundPixelTransform(agentCameraTr, groundZ);
  return findGroundPointForPixel(pixel, groundPixelTr);
}

Maybe<Vector3d> Spatialiser::findGroundPointForPixel(Vector2d const& pixel, Matrix3d const& groundPixelTr) const
{
  auto b = Vector3d{pixel(0) - d_cameraModel->imageWidth() / 2.0,
                    pixel(1) - d_cameraModel->imageHeight() / 2.0,
                    1};

  auto pt = groundPixelTr * b;

  if (pt(2) < 0)
    return Maybe<Vector3d>::empty();
  else
  {
    auto point = Vector3d{pt(0) / pt(2), pt(1) / pt(2), 0.0};
    return Maybe<Vector3d>{point};
  }
}


pair<MatrixXd, VectorXi> Spatialiser::findGroundPountsForPixels(MatrixXd const& pixels,
                                                                double groundZ) const
{
}

pair<MatrixXd, VectorXi> Spatialiser::findGroundPountsForPixels(MatrixXd const& pixels,
                                                                Affine3d const& agentCameraTr,
                                                                double groundZ) const
{
}

pair<MatrixXd, VectorXi> Spatialiser::findGroundPountsForPixels(MatrixXd const& pixels,
                                                                Matrix3d const& groundPixelTr) const
{
  auto b = (MatrixXd{3, pixels.cols()} << pixels, VectorXd::Zero(pixels.cols())).finished();
  auto pts = groundPixelTr * b;
  //pts.colwise()
}

