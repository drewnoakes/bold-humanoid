#pragma once

#include <stdlib.h>
#include <iostream>

#ifdef __GNUC__
  #define likely(x)      __builtin_expect(!!(x), 1)
  #define unlikely(x)    __builtin_expect(!!(x), 0)
#else
  #define likely(x)      (x)
  #define unlikely(x)    (x)
#endif

/// ASSERT(expr) checks if expr is true.  If not, error details are logged
/// and the process is exited with a non-zero code.
#ifdef INCLUDE_ASSERTIONS
#define ASSERT(expr)                                                      \
    if (unlikely(!(expr))) {                                              \
        char buf[4096];                                                   \
        snprintf (buf, 4096, "Assertion failed in \"%s\", line %d\n%s\n", \
                 __FILE__, __LINE__, #expr);                              \
        std::cerr << buf;                                                 \
        ::abort();                                                        \
    }                                                                     \
    else // This 'else' exists to catch the user's following semicolon
#else
#define ASSERT(expr)
#endif


/// DASSERT(expr) is just like ASSERT, except that it only is functional in
/// DEBUG mode, but does nothing when in a non-DEBUG build.
#ifdef DEBUG
# define DASSERT(expr) ASSERT(expr)
#else
# define DASSERT(expr) /* DASSERT does nothing when not debugging */
#endif
