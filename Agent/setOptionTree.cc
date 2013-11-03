#include "agent.ih"

void Agent::setOptionTree(unique_ptr<OptionTree> tree)
{
  cout << "[Agent::setOptionTree] Setting OptionTree with " << tree->optionCount() << " options" << endl;
  d_optionTree = move(tree);
}
