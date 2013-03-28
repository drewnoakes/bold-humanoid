#include "agentmodel.ih"

void AgentModel::updatePosture()
{
  list<shared_ptr<BodyPart>> partQueue;

  d_torso->transform.setIdentity();

  partQueue.push_back(d_torso);

  while (!partQueue.empty())
  {
    shared_ptr<BodyPart> part = partQueue.front();
    partQueue.pop_front();

    // Limb: Determine transformation of all connected joints by
    // applying proper translation
    if (shared_ptr<Limb> limb = dynamic_pointer_cast<Limb>(part))
    {
      // Loop over all joints
      for (auto joint : limb->joints)
      {
        // Transformation = that of limb plus translation to the joint
        joint->transform =
          limb->transform *
          Translation3d(joint->anchors.first);

        partQueue.push_back(joint);
      }
    }
    else
    {
      shared_ptr<Joint> joint = dynamic_pointer_cast<Joint>(part);

      // NOTE we don't update for joints with negative IDs, as these are fixed (a bit hacky)
      if (joint->id >= 0)
      {
        joint->angle = mx28States[joint->id].presentPosition;
      }

      shared_ptr<BodyPart> part2 = joint->bodyPart;
      part2->transform = joint->transform
        * AngleAxisd(joint->angle, joint->axis)
        * Translation3d(-joint->anchors.second);

      partQueue.push_back(part2);
    }
  }
}