#include "lookaround.ih"

std::vector<std::shared_ptr<Option>> LookAround::runPolicy()
{
  // Make an oscillatory movement to search for the ball

  double t = Clock::getSeconds();

  double durationHoriz = d_durationHoriz->getValue();
  double durationVert = d_durationVert->getValue();

  if (t - d_lastTimeSeconds > 1)
  {
    // It's been long enough since we last ran that we consider this a re-start.

    // Start quarter-way through the first phase, so the head is slightly
    // to the left, and pans right through the top of the box.
    d_startTimeSeconds = t - (durationHoriz/4.0);
  }

  d_lastTimeSeconds = t;

  double period = (durationHoriz + durationVert) * 2;

  double phase = fmod(t - d_startTimeSeconds, period);

  double panDegs = 0;
  double tiltDegs = 0;

  assert(phase >= 0);

  if (phase < durationHoriz)
  {
    // moving right-to-left across top
    tiltDegs = d_topAngle->getValue();
    panDegs = Math::lerp(phase/durationHoriz, -d_sideAngle, d_sideAngle);
  }
  else
  {
    phase -= durationHoriz;

    if (phase < durationVert)
    {
      // moving top-to-bottom at left
      tiltDegs = Math::lerp(phase/durationVert, d_topAngle->getValue(), d_bottomAngle->getValue());
      panDegs = d_sideAngle;
    }
    else
    {
      phase -= durationVert;

      if (phase < durationHoriz)
      {
        // moving left-to-right across bottom
        tiltDegs = d_bottomAngle->getValue();
        panDegs = Math::lerp(phase/durationHoriz, d_sideAngle, -d_sideAngle);
      }
      else
      {
        phase -= durationHoriz;

        if (phase < durationVert)
        {
          // moving bottom-to-top at right
          tiltDegs = Math::lerp(phase/durationVert, d_bottomAngle->getValue(), d_topAngle->getValue());
          panDegs = -d_sideAngle;
        }
        else
        {
          cout << "[LookAround::runPolicy] Failed to find phase of motion" << endl;
        }
      }
    }
  }

  // Move to the calculated position
  d_headModule->moveToDegs(panDegs, tiltDegs);

  return std::vector<std::shared_ptr<Option>>();
}
