#pragma once

namespace bold
{
  class ArmSection;
  class HeadSection;
  class LegSection;

  class PoseProvider
  {
  public:
    virtual void applyHead(HeadSection* head) = 0;
    virtual void applyArms(ArmSection* arms) = 0;
    virtual void applyLegs(LegSection* legs) = 0;
  };
}
