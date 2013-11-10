#include "optiontree.ih"

void OptionTree::run()
{
  vector<shared_ptr<Option>> ranOptions;

  list<shared_ptr<Option>> queue = {d_top};

  while (!queue.empty())
  {
    // Pop the top option from the queue
    shared_ptr<Option> option = queue.front();
    queue.pop_front();

//     cout << "Running option <" << option->getID() << ">" << endl;

    // Run it
    vector<shared_ptr<Option>> subOptions = option->runPolicy();

    // Push any suboptions it created onto the back of the queue
    queue.insert(queue.end(), subOptions.begin(), subOptions.end());

    // Remember the fact that we ran it
    ranOptions.push_back(option);
  }

  AgentState::getInstance().set(make_shared<OptionTreeState const>(ranOptions));
}
