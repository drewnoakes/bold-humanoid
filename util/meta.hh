#pragma once

namespace bold
{
  template <bool COND, int A, int B>
  struct IF
  {
    static const int value = A;
  };
  
  template <int A, int B>
  struct IF<false, A, B>
  {
    static const int value = B;
  };
  
  template <int A, int B>
  struct MIN : IF<A < B, A, B>
  {};

  template<int Index, class Search, class First, class... Types>
  struct get_internal
  {
    typedef typename get_internal<Index + 1, Search, Types...>::type type;
    static constexpr int index = Index;
  };

  template<int Index, class Search, class... Types>
  struct get_internal<Index, Search, Search, Types...>
  {
    typedef get_internal type;
    static const int index = Index;
  };

  template<class T, class... Types>
  T get(std::tuple<Types...> const& tuple)
  {
    return std::get<get_internal<0,T,Types...>::type::index>(tuple);
  }
}
