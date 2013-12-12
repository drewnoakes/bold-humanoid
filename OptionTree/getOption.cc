#include "optiontree.ih"

shared_ptr<Option> OptionTree::getOption(string const& id) const
{
  auto option = d_options.find(id);
  if (option == d_options.end())
  {
    log::error("OptionTree::getOption") << "Option '" << id << "' not found!";
    return nullptr;
  }

  return option->second;
}
