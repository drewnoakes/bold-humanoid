#ifndef BATS_DISJOINTSET_HH
#define BATS_DISJOINTSET_HH

#include <map>
#include <set>
#include <vector>
#include <algorithm>

namespace bats
{
  template <typename T>
  class DisjointSet
  {
  public:

    DisjointSet() {}
    
    void insert(T const& el)
    {
      std::size_t idx = d_equivList.size();
      d_elementIdxMap[el] = idx;
      d_equivList.push_back(idx);
    }

    std::size_t find(T const& el)
    {
      return find(d_elementIdxMap[el]);
    }

    void merge(T const& el1, T const& el2)
    {
      merge(d_elementIdxMap[el1], d_elementIdxMap[el2]);
    }

    std::set<std::set<T>> getSubSets()
    {
      flattenEquivList();
      std::map<std::size_t, std::set<T>> subSetsM;

      for (auto elIdxPair : d_elementIdxMap)
      {
	std::size_t ssId = find(elIdxPair.second);
	if (subSetsM.find(ssId) == subSetsM.end())
	  subSetsM[ssId] = std::set<T>();
	subSetsM[ssId].insert(elIdxPair.first);
      }

      std::set<std::set<T>> subSets;
      std::transform(subSetsM.begin(), subSetsM.end(),
		     std::inserter(subSets, subSets.begin()),
		     [](std::pair<const size_t, std::set<T>> const& kv) { return kv.second; });
      return subSets;
    }

    std::size_t size() const;
  private:

    std::size_t find(std::size_t idx)
    {
      if (d_equivList[idx] != idx)
	d_equivList[idx] = find(d_equivList[idx]);
      return d_equivList[idx];
    }

    std::size_t merge(std::size_t idx1, std::size_t idx2)
    {
      std::size_t ss1 = find(idx1);
      std::size_t ss2 = find(idx2);
      
      if (ss1 < ss2)
	d_equivList[ss2] = ss1;
      else
	d_equivList[ss1] = ss2;
    }

    void flattenEquivList()
    {
      for (std::size_t idx = 0; idx < d_equivList.size(); ++idx)
	d_equivList[idx] = d_equivList[d_equivList[idx]];
    }

    std::map<T, std::size_t> d_elementIdxMap;
    std::vector<std::size_t> d_equivList;
  };
}

#endif
