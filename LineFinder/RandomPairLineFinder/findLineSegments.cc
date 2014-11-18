#include "randompairlinefinder.hh"

#include "../../geometry/LineSegment/LineSegment2/LineSegment2i/linesegment2i.hh"

using namespace bold;
using namespace std;
using namespace Eigen;

vector<LineSegment2i> RandomPairLineFinder::findLineSegments(vector<Vector2i>& lineDots)
{
  vector<LineHypothesis> hypotheses;

  // shuffle lineDots to simulate drawing at random
  random_shuffle(lineDots.begin(), lineDots.end());

  // TODO take processDotCount from a field
  const ushort processDotCount = 5000;

  int dotIndex = (int)min((size_t)processDotCount + 1, lineDots.size()) - 1;

  while (dotIndex > 1)
  {
    auto dot1 = lineDots[dotIndex--];
    auto dot2 = lineDots[dotIndex];

    if (abs(dot1.x() - dot2.x()) + abs(dot1.y() - dot2.y()) < static_cast<int>(d_minDotManhattanDistance))
    {
      // Ignore very short lines as they are indistinguishable from noise
      continue;
    }

    auto segment = LineSegment2i(dot1, dot2);
    auto line = Line::fromSegment(segment);

    // Do we have an entry already for this?
    bool found = false;
    for (LineHypothesis& hypothesis : hypotheses)
    {
      if (hypothesis.tryMerge(line, dot1, dot2))
      {
        found = true;
        break;
      }
    }

    // If not, create one
    if (!found)
    {
      hypotheses.emplace_back(line, dot1, dot2);
    }
  }

  if (hypotheses.size() == 0)
    return vector<LineSegment2i>();

  // Sort highest-voted first
  sort(hypotheses.begin(), hypotheses.end(), [](LineHypothesis const& a, LineHypothesis const& b) { return a.count() > b.count(); });

  // Calculate the average vote count for the top N hypotheses
  int sumVotes = 0;
  int takeTop = 0;
  for (auto const& hypothesis : hypotheses)
  {
    if (++takeTop == 15) // <-- controllable number
      break;
    sumVotes += hypothesis.count();
  }
  ASSERT(takeTop != 0);
  int averageVotes = sumVotes / takeTop;

  vector<LineSegment2i> satisfactory;
  for (LineHypothesis const& hypothesis : hypotheses)
  {
    if (hypothesis.lengthDistribution().average() / hypothesis.lengthDistribution().stdDev() > 2.0)
      continue;

    // Only take those with an above average number of votes
    if (hypothesis.count() < averageVotes)
      break;

    if (hypothesis.min() == hypothesis.max())
      break;

    satisfactory.emplace_back(
      Vector2i(hypothesis.min().x(), hypothesis.min().y()),
      Vector2i(hypothesis.max().x(), hypothesis.max().y())
    );
  }
  return satisfactory;
}
