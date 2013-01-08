#include "../DisjointSet/disjointset.hh"
#include <iostream>

using namespace bats;
using namespace std;

#define N 100000

int main()
{
  DisjointSet<int> set;

  auto unionPred = [](int* a, int* b) { return (*a % 3) == (*b % 3); };
  vector<int> ints(N);
  iota(ints.begin(), ints.end(), 1);
  random_shuffle(ints.begin(), ints.end());

  DisjointSet<int>::Element* elems[N];

  for (unsigned i = 0; i < N; ++i) {
    elems[i] = set.insert(&ints[i], unionPred);
  }

  
  auto elements = set.getElements();
  cout << "Number of elements: " << elements.size() << endl;

  auto subsets = set.getSubSets();

  cout << "Number of subsets: " << subsets.size() << endl;
}
