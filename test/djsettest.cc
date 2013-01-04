#include "../DisjointSet/disjointset.hh"
#include <iostream>

using namespace bats;
using namespace std;

int main()
{
  DisjointSet<int> set;

  auto unionPred = [](int* a, int* b) { return (*a - *b) % 2 == 0; };
  int ints[] = {1,2,3,4,5,6};
  DisjointSet<int>::Element* elems[6];

  for (unsigned i = 0; i < 6; ++i) {
    elems[i] = set.insert(ints + i, unionPred);
    cout << *(elems[i]->parent->obj) << endl;
  }

  
  auto elements = set.getElements();
  cout << "Number of elements: " << elements.size() << endl;
  for (unsigned i = 0; i < 6; ++i) {
    cout << *(elements[i]->obj) << endl;
  }

  auto subsets = set.getSubSets();

  cout << "Number of subsets: " << subsets.size() << endl;
}
