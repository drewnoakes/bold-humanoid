#include "optiontree.ih"

void OptionTree::run()
{
  d_ranOptions.clear();

  OptionList options = {d_top};
  while (!options.empty())
  {
    OptionPtr option = options.front();
    cout << "Running option <" << option->getID() << ">" << endl;
    options.pop_front();
    OptionList subOptions = option->runPolicy();
    options.insert(options.end(), subOptions.begin(), subOptions.end());
    d_ranOptions.push_back(option);
  }
}
