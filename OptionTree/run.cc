#include "optiontree.ih"

void OptionTree::run()
{
  OptionList ranOptions;

  list<OptionPtr> queue = {d_top};

  while (!queue.empty())
  {
    // Pop the top option from the queue
    OptionPtr option = queue.front();
    queue.pop_front();

//     cout << "Running option <" << option->getID() << ">" << endl;

    // Run it
    OptionList subOptions = option->runPolicy();

    // Push any suboptions it created onto the back of the stack
    queue.insert(queue.end(), subOptions.begin(), subOptions.end());

    // Remember the fact that we ran it
    ranOptions.push_back(option);
  }

  AgentState::getInstance().set(make_shared<OptionTreeState const>(ranOptions));
}
