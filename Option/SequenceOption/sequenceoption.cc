#include "sequenceoption.hh"

#include "../util/log.hh"

using namespace bold;
using namespace std;
using namespace rapidjson;

SequenceOption::SequenceOption(std::string const& id, std::initializer_list<std::shared_ptr<Option>> sequence)
: Option(id, "SequenceOption"),
  d_sequence(sequence),
  d_index(-1)
{
  if (sequence.size() == 0)
  {
    log::error("SequenceOption::SequenceOption") << "Must specify at least one option in sequence: " << id;
    throw new runtime_error("Must specify at least one option in the sequence");
  }
}

void SequenceOption::reset()
{
  d_index = -1;
}

double SequenceOption::hasTerminated()
{
  return d_index == static_cast<int>(d_sequence.size()) ? 1.0 : 0.0;
}

vector<shared_ptr<Option>> SequenceOption::runPolicy(Writer<StringBuffer>& writer)
{
  if (d_index == -1)
  {
    auto option = d_sequence[0];
    option->reset();
    return {option};
  }

  while (true)
  {
    if (d_index == static_cast<int>(d_sequence.size()))
      return {};

    auto option = d_sequence[d_index];

    if (option->hasTerminated() == 1.0)
    {
      d_index++;
    }
    else
    {
      return {option};
    }
  }
}
