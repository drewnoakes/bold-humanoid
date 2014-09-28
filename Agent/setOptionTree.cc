#include "agent.ih"

void Agent::setOptionTree(shared_ptr<OptionTree> tree)
{
  log::verbose("Agent::setOptionTree") << "Setting OptionTree";
  d_optionTree = tree;
  d_streamer->setOptionTree(tree);
}
