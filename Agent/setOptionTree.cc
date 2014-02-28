#include "agent.ih"

void Agent::setOptionTree(shared_ptr<OptionTree> tree)
{
  log::info("Agent::setOptionTree") << "Setting OptionTree with " << tree->optionCount() << " options";
  d_optionTree = tree;
  d_streamer->setOptionTree(tree);
}
