#include "../DisjointSet/disjointset.hh"
#include <iostream>
#include <cmath>

using namespace bold;
using namespace std;

#define N 1000000

bool flipCoin()
{
  return (rand() % 20) == 0;
}

int main()
{
  DisjointSet<int> set;

  vector<int> ints(N);
  iota(ints.begin(), ints.end(), 1);
  random_shuffle(ints.begin(), ints.end());

  int someSeenEven = -1;
  int someSeenOdd = -1;
  for (unsigned i = 0; i < N; ++i)
  {
    set.insert(ints[i]);
    
    if (ints[i] % 2 == 0)
    {
      if (someSeenEven != -1)
	set.merge(ints[i], someSeenEven);
      if (someSeenEven == -1 || flipCoin())
	someSeenEven = ints[i];
    }

    if (ints[i] % 2 != 0)
    {
      if (someSeenOdd != -1)
	set.merge(ints[i], someSeenOdd);
      if (someSeenOdd == -1 || flipCoin())
	someSeenOdd = ints[i];
    }
  }

  auto subSets = set.getSubSets();
  cout << "Number of subsets: " << subSets.size() << endl;
  for (auto subSet : subSets)
    cout << "Size: " << subSet.size() << endl;

/*
  
  auto elements = set.getElements();
  cout << "Number of elements: " << elements.size() << endl;

  auto subsets = set.getSubSets();

  cout << "Number of subsets: " << subsets.size() << endl;
*/
}
