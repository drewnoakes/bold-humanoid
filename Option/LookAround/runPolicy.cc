#include "lookaround.ih"

std::vector<std::shared_ptr<Option>> LookAround::runPolicy()
{
  // Make an oscillatory movement to search for the ball

  double t = Clock::getSeconds();

  if (t - d_lastTimeSeconds > 1)
  {
    // It's been long enough since we last ran that we consider this a re-start.
    
    // Start quarter-way through the first phase, so the head is slightly
    // to the left, and pans right through the top of the box.
    d_startTimeSeconds = t - (d_durationHoriz/4.0);
  }

  d_lastTimeSeconds = t;

  double period = (d_durationHoriz + d_durationVert) * 2;

  double phase = fmod(t - d_startTimeSeconds, period);

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
          // moving bottom-to-top at right
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
  d_headModule->moveToDegs(hAngle, vAngle);

  return std::vector<std::shared_ptr<Option>>();
}
