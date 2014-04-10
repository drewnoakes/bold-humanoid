#include "visualcortex.ih"

Maybe<Vector2d> VisualCortex::detectBall(vector<Blob>& ballBlobs, SequentialTimer& t)
{
  Maybe<Vector2d> ballPosition = Maybe<Vector2d>::empty();

  if (d_ballBlobMergingEnabled->getValue())
  {
    // Merge ball blobs
    for (int i = 0; i < min(10, (int)ballBlobs.size()); ++i)
    {
      Blob& larger = ballBlobs[i];

      if (larger.area == 0)
        continue;

      if (larger.area < unsigned(d_minBallAreaPixels->getValue()))
      {
        // Blobs are sorted, largest first, so if this is too small, the rest will be too
        break;
      }

      for (int j = i + 1; j < min(10, (int)ballBlobs.size()); ++j)
      {
        Blob& smaller = ballBlobs[j];

        if (smaller.area == 0)
          continue;

        if (shouldMergeBallBlobs(larger.bounds(), smaller.bounds()))
        {
          larger.merge(smaller);
          // Indicate that the smaller one is no longer in use
          smaller.area = 0;
        }
      }
    }
    t.timeEvent("Ball Blob Merging");
  }

  // The first is the biggest, topmost ball blob
  auto ballPositionCandidates = vector<pair<Vector2d, Vector3d>>();

  // Filter out invalid ball blobs
  Vector2d imagePos;
  Vector3d agentFramePos;
  for (Blob const& ballBlob : ballBlobs)
    if (canBlobBeBall(ballBlob, imagePos, agentFramePos))
      ballPositionCandidates.push_back(make_pair(imagePos, agentFramePos));
      
  // Take the ball that is closest
  if (ballPositionCandidates.size() == 0)
    ballPosition = ballPosition.empty();
  else
  {
    auto nearest = min_element(begin(ballPositionCandidates), end(ballPositionCandidates),
                               [](pair<Vector2d, Vector3d> const& pos1, pair<Vector2d, Vector3d> const& pos2) {
                                 return pos1.second.head<2>().norm() < pos2.second.head<2>().norm();
                               });
    ballPosition = Maybe<Vector2d>(nearest->first);
  }
  t.timeEvent("Ball Blob Selection");

  return ballPosition;
}
