#include "option.ih"

Option::Option(std::string const& id)
  : Configurable(std::string("option.") + id),
    d_id(id)
{
  cout << "Creating option: " << id << endl;
}

Option::~Option()
{
  cout << "Destroying option: " << getID() << endl;
}
