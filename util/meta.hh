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
 }
