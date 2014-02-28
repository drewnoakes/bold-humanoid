#include "optiontree.ih"

#include "../Option/FSMOption/fsmoption.hh"

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

vector<shared_ptr<FSMOption>> OptionTree::getFSMs() const
{
  vector<shared_ptr<FSMOption>> fsmOptions;
  for (auto const& option : d_options)
  {
    auto fsmOption = dynamic_pointer_cast<FSMOption>(option.second);

    if (fsmOption)
      fsmOptions.push_back(fsmOption);
  }
  return fsmOptions;
}
