#include "spatialiser.hh"

#include "../AgentState/agentstate.hh"
#include "../StateObject/BodyState/bodystate.hh"
#include "../StateObject/AgentFrameState/agentframestate.hh"
#include "../StateObject/CameraFrameState/cameraframestate.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

void Spatialiser::updateCameraToAgent()
{
  auto body = AgentState::getInstance().get<BodyState>();
  auto cameraFrame = AgentState::getInstance().get<CameraFrameState>();

  /*
  auto neck = body->getLimb("neck");
  auto neckHeadJoint = neck->joints[0];
  auto head = body->getLimb("head");
  auto cameraJoint = head->joints[0];
  auto camera = body->getLimb("camera");
  auto lFoot = body->getLimb("lFoot");
  auto lKnee = body->getLimb("lLowerLeg");
  auto rFoot = body->getLimb("rFoot");

  cout << "---------------" << endl;
  cout << "neckHeadJoint: " << neckHeadJoint->angle << endl << neckHeadJoint->transform.translation().transpose() << endl;
  cout << "head:" << endl << head->transform.translation().transpose() << endl;
  cout << "cameraJoint:" << endl << cameraJoint->transform.translation().transpose() << endl;
  cout << "camera:" << endl << camera->transform.matrix() << endl;
  cout << "foot: " << endl << lFoot->transform.matrix() << endl;
  auto cameraToLFoot = lFoot->transform.inverse() * camera->transform;
  cout << "cam2foot: " << endl << cameraToLFoot.translation().transpose() << endl;

  cout << "l foot transform:" << endl << lFoot->transform.matrix() << endl;
  cout << "l knee transform:" << endl << lKnee->transform.matrix() << endl;
  cout << "r foot transform:" << endl << rFoot->transform.matrix() << endl;
  */

  // Multiplying with this transform brings coordinates from camera space in torso space
  auto cameraTransform = body->getLimb("camera")->transform;

  double torsoHeight = body->getTorsoHeight();

  auto const& ballObs = cameraFrame->getBallObservation();

  Maybe<Vector3d> ball = ballObs.hasValue()
    ? findGroundPointForPixel(ballObs->cast<int>(), torsoHeight, cameraTransform)
    : Maybe<Vector3d>::empty();

  std::vector<Vector3d> goals;
  std::vector<LineSegment3d> lineSegments;

  for (Vector2f const& goal : cameraFrame->getGoalObservations())
  {
    auto const& pos3d = findGroundPointForPixel(goal.cast<int>(), torsoHeight, cameraTransform);
    if (pos3d.hasValue())
    {
      goals.push_back(*pos3d.value());
    }
  }

  for (LineSegment2i const& lineSegment : cameraFrame->getObservedLineSegments())
  {
    auto const& p1 = findGroundPointForPixel(lineSegment.p1().cast<int>(), torsoHeight, cameraTransform);
    auto const& p2 = findGroundPointForPixel(lineSegment.p2().cast<int>(), torsoHeight, cameraTransform);
    if (p1.hasValue() && p2.hasValue())
    {
      lineSegments.push_back(LineSegment3d(*p1.value(), *p2.value()));
    }
  }

  AgentState::getInstance().set(make_shared<AgentFrameState const>(ball, goals, lineSegments));
}
