#include <gtest/gtest.h>

#include "helpers.hh"

#include <Eigen/Core>
#include <Eigen/Geometry>

using namespace std;
using namespace Eigen;

const Vector3d origin(0, 0, 0);

TEST (EigenTests, transforms)
{
  // Combine multiple translations, rotations (and scalings) by multiplying
  // them together.

  // Rotate around X by 90 degrees, then transform 1 unit along +Z (which is
  // now pointing along -Y from the original frame due to the rotation.)

  Affine3d transform(AngleAxisd(M_PI/2, Vector3d::UnitX()) * Translation3d(0, 0, 1));

  // Use the right-hand-rule to determine the direction of rotations.

  // Matrix multiplication is not commutative. Multiplying a set of transforms
  // in different orders gives different results.

  // Calculate the transform's effect on a point in space by multiplying the
  // transform by the vector.

  // Eigen's vector types are columns. They may only be post-multiplied with
  // transformation matrices.

  // Even though a transformation matrix may be both pre and post multiplied
  // with a vector, Eigen only supports the latter.
  // You multiply T*V. V*T will not compile.

  EXPECT_TRUE ( VectorsEqual ( Vector3d(0, -1, 0), transform * origin ) );

  // The transform not only moves points, but rotates them.
  // Transforming non-origin values will demonstrate this.

  EXPECT_TRUE ( VectorsEqual ( Vector3d(0, -1, 1), transform * Vector3d::UnitY() ) );

  // It's more useful to think of transforms as operating upon coordinate
  // frames rather than points. Transforms map points in one coordinate space
  // to another.

  // Consider a transform from the agent's torso frame to the frame of the
  // left ankle. This simple example only involves translation, but the following
  // discussion holds in the presence of rotations too.

  Affine3d torsoLeftAnkle(Translation3d(
    -0.074/2,
    -0.005,
    -0.1222 - 0.093 - 0.093));

  // Multiplying by the origin gives:

  EXPECT_TRUE ( VectorsEqual ( Vector3d(-0.074/2, -0.005, -0.1222 - 0.093 - 0.093),
                               torsoLeftAnkle * origin ) );

  // In this case, the vector multiplied should be considered in the 'ankle' frame.
  // The result is position of the same point in the 'torso' frame.

  // A convention we use in the code is to name transforms "AB", where A is one
  // space, and B is another.

  // In this way:
  //   AB * Vb = Va

  // `torsoLeftAnkle * origin` gives the position of the ankle's origin in
  // the torso's frame:

  // A transform may be inverted.
  //   BA = AB.inverse()

  Affine3d leftAnkleTorso(torsoLeftAnkle.inverse());

  // `leftAnkleTorso * origin` gives the position of the torso's origin in
  // the left ankle's frame:

  EXPECT_TRUE ( VectorsEqual ( Vector3d(0.074/2, 0.005, 0.1222 + 0.093 + 0.093),
                               leftAnkleTorso * origin ) );

  // The transform may be deconstructed into translation and rotation components

  EXPECT_TRUE ( VectorsEqual ( Vector3d(0, -1, 0), transform.translation() ) );

  EXPECT_TRUE ( MatricesEqual ( AngleAxisd(M_PI/2, Vector3d::UnitX()).matrix(),
                                transform.rotation() ) );
}
