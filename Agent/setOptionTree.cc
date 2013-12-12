#include "agent.ih"

void Agent::setOptionTree(unique_ptr<OptionTree> tree)
{
  log::info("Agent::setOptionTree") << "Setting OptionTree with " << tree->optionCount() << " options";
  d_optionTree = move(tree);
}
