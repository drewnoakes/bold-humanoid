#pragma once

namespace bold
{
  template <bool COND, int A, int B>
  struct IF
  {
    enum { val = A };
  };
  
  template <int A, int B>
  struct IF<false, A, B>
  {
    enum { val = B };
  };
  
  template <int A, int B>
  struct MIN : IF<A < B, A, B>
  {};
}
