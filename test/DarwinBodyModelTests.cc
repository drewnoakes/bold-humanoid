#include "gtest/gtest.h"

#include "../BodyModel/DarwinBodyModel/darwinbodymodel.hh"

using namespace std;
using namespace bold;

TEST (DarwinBodyModelTests, totalMass)
{
  DarwinBodyModel model;
  double totalMass = 0.0;
  model.visitLimbs([&](shared_ptr<Limb const> const& limb) { totalMass += limb->mass; });

  EXPECT_NEAR ( 2.92533, totalMass, 0.00001 );
}
