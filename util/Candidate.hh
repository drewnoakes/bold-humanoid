#pragma once

namespace bold
{
  template<typename T>
  struct Candidate
  {
  public:
    T item() const { return d_item; }
    int votes() const { return d_votes; }

    Candidate(T item, int votes)
    : d_item(item),
      d_votes(votes)
    {}

  private:
    T d_item;
    int d_votes;
  };
}
