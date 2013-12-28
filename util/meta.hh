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


    template<typename ...>
    struct for_each_impl
    {};

    template<typename T, typename F, typename... Pre, typename... ArgTypes>
    struct for_each_impl<T, F, std::tuple<Pre...>, std::tuple<>, ArgTypes... >
    {
      static void do_it(T const& tuple, ArgTypes... args) {}
    };

    template<typename T, typename F, typename... Pre, typename Cur, typename... Post, typename... ArgTypes>
    struct for_each_impl<T, F, std::tuple<Pre...>, std::tuple<Cur,Post...>, ArgTypes...>
    {
      static void do_it(T const& tuple, ArgTypes... args)
      {
        F::do_it(get<Cur>(tuple), args...);
        for_each_impl<T, F, std::tuple<Pre...,Cur>,std::tuple<Post...>, ArgTypes... >::do_it(tuple, args...);
      }
    };

    template<typename F, typename... Types, typename... ArgTypes>
    void for_each(std::tuple<Types...> const& tuple, ArgTypes... args)
    {
      for_each_impl<std::tuple<Types...>, F, std::tuple<>, std::tuple<Types...>, ArgTypes... >::do_it(tuple, args...);
    }
  }
}
