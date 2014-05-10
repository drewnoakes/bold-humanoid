#pragma once

namespace bold
{
  /** Compile time test whether a type has a \a fill member
   *
   * Works thanks to SFINAE (Substitution Failure Is Not An Error): if
   * U::fill does not exist, \a decltype does not give a valid type to
   * substitute as an argument, which removes the first template
   * method from valid options. Otherwise, it passes and is chosen as
   * being more specific than the second overload.
   */
  template<typename T>
  struct has_fill_member
  {
    typedef char yes;
    typedef long no;

    template<typename U>
    static yes test(decltype(&U::fill));

    template<typename U>
    static no test(...);

    static const bool value = sizeof(test<T>(0)) == sizeof(yes);
  };

  /// Traits used to select correct method of zero-ing the used type
  struct AveragingTraits
  {
    template<typename T>
    static void zero(T& v, typename std::enable_if<has_fill_member<T>::value>::type* dummy = 0) { v.fill(0); }

    template<typename T>
    static void zero(T& v, typename std::enable_if<std::is_arithmetic<T>::value>::type* dummy = 0) { v = 0; }
  };
}
