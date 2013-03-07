#include "linefinder.hh"

using namespace bold;
using namespace std;
using namespace Eigen;

vector<LineFinder::LineHypothesis> LineFinder::find(vector<Vector2i>& lineDots, ushort processDotCount)
{
  vector<LineHypothesis> hypotheses;

  // shuffle lineDots to simulate drawing at random
  random_shuffle(lineDots.begin(), lineDots.end());

  int dotIndex = min((size_t)processDotCount, lineDots.size() - 1);

  while (dotIndex > 1)
  {
    auto dot1 = lineDots[dotIndex--];
    auto dot2 = lineDots[dotIndex--];

    if (dot1.x() == dot2.x() && dot1.y() == dot2.y())
      continue;

    auto segment = LineSegment2i(dot1, dot2);
    auto line = segment.toLine();

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
      hypotheses.push_back(LineHypothesis(line, dot1, dot2));
    }
  }

  // Sort highest-voted first
  sort(hypotheses.begin(), hypotheses.end(), [](LineHypothesis const& a, LineHypothesis const& b) { return a.count() > b.count(); });

  return hypotheses;
}