#include "optiontree.ih"

shared_ptr<Option> OptionTree::getOption(string const& id) const
{
  auto option = d_options.find(id);
  if (option == d_options.end())
  {
    cout << "[OptionTree] Option " << id << " not found!" << endl;
    return 0;
  }

  return option->second;
}

