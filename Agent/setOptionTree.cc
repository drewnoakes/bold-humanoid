#include "agent.ih"

void Agent::setOptionTree(unique_ptr<OptionTree> tree)
{
  cout << "[Agent] Got optiontree, with nr of options: " << tree->optionCount() << endl;
  d_optionTree = move(tree);
}
