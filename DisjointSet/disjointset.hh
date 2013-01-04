#ifndef BATS_DISJOINTSET_HH
#define BATS_DISJOUNTSET_HH

#include <vector>
#include <functional>
#include <algorithm>
#include <iostream>

namespace bats
{
  template <typename T>
  class DisjointSet
  {
  public:
    struct Element
    {
      T* obj;
      Element* parent;

      Element(T* o)
      : obj(o)
      {}
    };

    /** Make a new singleton set
     *
     * Creates a new subset containing only @a el
     */
    void makeNewSet(Element* el)
    {
      el->parent = el;
    }

    /** Find containing set
     *
     * Find the subset containting @a el
     * @returns the representative element of the subset
     */
    Element* findSet(Element* el)
    {
      if (el->parent != el)
	el->parent = findSet(el->parent);
      return el->parent;
    }

    /** Union two subsets
     *
     * Finds the subsets that contain @a e1 and @e2 respectively, and
     * performs a union to join them together.
     */
    void unionSets(Element* e1, Element* e2)
    {
      Element* r1 = findSet(e1);
      Element* r2 = findSet(e2);
      
      if (r1 == r2)
	return;

      if (r1 < r2)
	r2->parent = r1;
      else
	r1->parent = r2;
    }

    /** Insert an object
     *
     * Inserts @a obj as a new singleton subset.
     */
    Element* insert(T* obj)
    {
      Element* e = new Element(obj);
      makeNewSet(e);
      d_elements.push_back(e);
      return e;
    }

    /** Insert an object with union
     *
     * Inserts @a obj, and checks whether to add it to an existing subset.
     */
    Element* insert(T* obj, std::function<bool(T*,T*)> unionPred) {
      Element* e1 = new Element(obj);
      makeNewSet(e1);
      for (Element* e2 : d_elements)
	if (unionPred(e1->obj, e2->obj))
	  unionSets(e1, e2);
      d_elements.push_back(e1);
      return e1;
    }

    std::vector<Element*> getElements() const
    {
      return d_elements;
    }

    std::vector<Element*> getSubSets()
    {
      std::vector<Element*> dest(d_elements.size());
      auto dend = std::remove_copy_if(d_elements.begin(), d_elements.end(),
			  dest.begin(),
			  [] (Element* el) { return el->parent != el; });
      dest.resize(dend - dest.begin());
      return dest;
    }

  private:
    std::vector<Element*> d_elements;
  };
}

#endif
