#include "option.hh"

#include <iostream>

using namespace bold;
using namespace std;

Option::Option(string const& id)
  : d_id(id)
{
  cout << "[Option::Option] Creating option: " << d_id << endl;
}

Option::~Option()
{
  cout << "[Option::~Option] Destroying option: " << d_id << endl;
}
