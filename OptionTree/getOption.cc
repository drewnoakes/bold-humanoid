#include "optiontree.ih"

#include "../Option/FSMOption/fsmoption.hh"


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
