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
      static void do_it(T const& tuple, F f, ArgTypes... args) {}
    };

    template<typename T, typename F, typename... Pre, typename Cur, typename... Post, typename... ArgTypes>
    struct for_each_impl<T, F, std::tuple<Pre...>, std::tuple<Cur,Post...>, ArgTypes...>
    {
      static void do_it(T const& tuple, F f, ArgTypes... args)
      {
        f(get<Cur>(tuple), args...);
        for_each_impl<T, F, std::tuple<Pre...,Cur>,std::tuple<Post...>, ArgTypes... >::do_it(tuple, f, args...);
      }
    };

    template<typename F, typename... Types, typename... ArgTypes>
    void for_each(std::tuple<Types...> const& tuple, F f, ArgTypes... args)
    {
      for_each_impl<std::tuple<Types...>, F, std::tuple<>, std::tuple<Types...>, ArgTypes... >::do_it(tuple, f, args...);
    }

    /*
    template<template <typename> class F>
    struct bind
    {
      template<typename T>
      struct apply
      {
        typedef F<T> type;
      };
    };


    template<typename ...>
    struct transform_impl
    {};

    template<typename Op, typename... Trans>
    struct transform_impl<Op, std::tuple<Trans...>, std::tuple<> >
    {
      typedef std::tuple<Trans...> type;
    };

    template<typename Op, typename... Trans, typename Cur, typename... Post>
    struct transform_impl<Op, std::tuple<Trans...>, std::tuple<Cur,Post...> >
      : transform_impl<Op, std::tuple<Trans...,typename Op::template apply<Cur>::type>, std::tuple<Post...> >
    {};

    template<typename...>
    struct transform {};

    template<typename Op, typename... Types>
    struct transform<Op, std::tuple<Types...> >
    {
      typedef typename transform_impl<Op, std::tuple<>, std::tuple<Types...> >::type type;
    };
    */
  }
}
