#include "agent.hh"

#include "../DataStreamer/datastreamer.hh"
#include "../OptionTree/optiontree.hh"

using namespace bold;
using namespace std;

void Agent::setOptionTree(shared_ptr<OptionTree> tree)
{
  log::verbose("Agent::setOptionTree") << "Setting OptionTree";
  d_optionTree = tree;
  d_streamer->setOptionTree(tree);
}
