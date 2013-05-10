#include "lookaround.ih"

OptionList LookAround::runPolicy()
{
  // Make an oscillatory movement to search for the ball

  double t = Clock::getSeconds();

  double period = (d_durationHoriz + d_durationVert) * 2;

  double phase = fmod(t, period);

  double hAngle = 0;
  double vAngle = 0;

  assert(phase >= 0);

  if (phase < d_durationHoriz)
  {
    // moving right-to-left across top
    vAngle = d_topAngle;
    hAngle = Math::lerp(phase/d_durationHoriz, -d_sideAngle, d_sideAngle);
  }
  else
  {
    phase -= d_durationHoriz;

    if (phase < d_durationVert)
    {
      // moving top-to-bottom at left
      vAngle = Math::lerp(phase/d_durationVert, d_topAngle, d_bottomAngle);
      hAngle = d_sideAngle;
    }
    else
    {
      phase -= d_durationVert;

      if (phase < d_durationHoriz)
      {
        // moving left-to-right across bottom
        vAngle = d_bottomAngle;
        hAngle = Math::lerp(phase/d_durationHoriz, d_sideAngle, -d_sideAngle);
      }
      else
      {
        phase -= d_durationHoriz;

        if (phase < d_durationVert)
        {
          // moving bottom-to-top across right
          vAngle = Math::lerp(phase/d_durationVert, d_bottomAngle, d_topAngle);
          hAngle = -d_sideAngle;
        }
        else
        {
          cout << "[LookAround::runPolicy] Failed to find phase of motion" << endl;
        }
      }
    }
  }

  // Move to the calculated position
//   Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);
  d_headModule->moveToAngle(hAngle, vAngle);

  return OptionList();
}
