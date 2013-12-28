#pragma once

namespace bold
{
  namespace meta
  {
    template <bool COND, int A, int B>
    struct if_
    {
      typedef if_<COND,A,B> type;
      static const int value = A;
    };
  
    template <int A, int B>
    struct if_<false, A, B>
    {
      typedef if_<false,A,B> type;
      static const int value = B;
    };
  
    template <int A, int B>
    struct min : if_<A < B, A, B>
    {};


    template<int Index, typename Search, typename First, typename... Types>
    struct get_internal
    {
      typedef typename get_internal<Index + 1, Search, Types...>::type type;
      static constexpr int index = Index;
    };

    template<int Index, typename Search, typename... Types>
    struct get_internal<Index, Search, Search, Types...>
    {
      typedef get_internal type;
      static constexpr int index = Index;
    };

    /** Get a tuple member by type
     *
     * Won't be needed anymore with C++14
     */
    template<typename T, typename... Types>
    T get(std::tuple<Types...> const& tuple)
    {
      return std::get<get_internal<0,T,Types...>::type::index>(tuple);
    }


    template <int I = 0, typename Function, typename... Types>
    typename std::enable_if<sizeof...(Types) == I, void>::type
    for_each(std::tuple<Types...> const& tuple, Function f)
    {}

    template <int I = 0, typename Function, typename... Types>
    typename std::enable_if<I < sizeof...(Types), void>::type
    for_each(std::tuple<Types...> const& tuple, Function f)
    {
      f(std::get<I>(tuple));
      for_each<I+1,Function,Types...>(tuple, f);
    }
  }
}
