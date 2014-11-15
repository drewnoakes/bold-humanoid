#include "labelcountpass.hh"

#include "../../PixelLabel/pixellabel.hh"
#include "../../SequentialTimer/sequentialtimer.hh"
#include "../../State/state.hh"
#include "../../StateObject/LabelCountState/labelcountstate.hh"

using namespace bold;
using namespace std;

LabelCountPass::LabelCountPass(std::vector<std::shared_ptr<PixelLabel>> const& labels)
  : d_countByLabelId(),
    d_labels(labels)
{}

void LabelCountPass::process(ImageLabelData const& labelData, SequentialTimer& timer)
{
  for (std::shared_ptr<PixelLabel> label : d_labels)
  {
    ASSERT((uint8_t)label->getID() < MAX_LABEL_COUNT);
    d_countByLabelId[(uint8_t)label->getID()] = 0;
  }
  timer.timeEvent("Clear");

  for (auto const& row : labelData)
  {
    for (auto const& label : row)
    {
      if (label != 0)
        d_countByLabelId[label]++;
    }
  }
  timer.timeEvent("Process Rows");

  std::map<std::shared_ptr<PixelLabel>,uint> counts;

  for (std::shared_ptr<PixelLabel> label : d_labels)
    counts[label] = d_countByLabelId[(uint8_t)label->getID()];

  State::make<LabelCountState>(counts);
}
