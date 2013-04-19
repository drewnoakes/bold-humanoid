#include "gtest/gtest.h"

#include "../util/Lazy.hh"

#include <memory>

using namespace bold;
using namespace std;

TEST (LazyTests, design)
{
  bool called = false;
  Lazy<int> lazy([&](){ called = true; return make_shared<int>(123); });

  EXPECT_FALSE( called );
  EXPECT_FALSE( lazy.hasValue() );

  auto value = lazy.value();

  EXPECT_TRUE( called );
  EXPECT_TRUE( lazy.hasValue() );

  EXPECT_EQ( 123, *value );
}

TEST (LazyTests, uninitialised)
{
  Lazy<int> lazy;

  EXPECT_FALSE(lazy.hasValue());

  EXPECT_EQ ( nullptr, lazy.value() );
}
