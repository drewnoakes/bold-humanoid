#include "optiontree.ih"

void OptionTree::run()
{
  OptionList ranOptions;

  OptionList options = {d_top};
  while (!options.empty())
  {
    OptionPtr option = options.front();
    cout << "Running option <" << option->getID() << ">" << endl;
    options.pop_front();
    OptionList subOptions = option->runPolicy();
    options.insert(options.end(), subOptions.begin(), subOptions.end());
    ranOptions.push_back(option);
  }

  auto s = make_shared<OptionTreeState const>(ranOptions);
  AgentState::getInstance().set(s);
}
