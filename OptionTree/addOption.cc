#include "optiontree.ih"

void OptionTree::addOption(shared_ptr<Option> option, bool top)
{
  d_options[option->getID()] = option;

  if (top)
  {
    assert(!d_top && "top option already added");
    d_top = option;
  }
}
