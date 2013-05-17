#include "gtest/gtest.h"

#include "helpers.hh"

#include <memory>

using namespace std;

class Base
{
public:
  virtual bool isSomething() = 0;
};

class Derived : public Base
{
public:
  bool called;
  bool isSomething() override { called = true; return true; }
};

TEST (CppTests, castingSharedPointers)
{
  auto derived = make_shared<Derived>();

  //
  // Ensure no slicing occurs
  //
  
  auto base = dynamic_pointer_cast<Base>(derived);
  
  EXPECT_TRUE( base->isSomething() );
  EXPECT_TRUE( derived->called );
  EXPECT_TRUE( dynamic_pointer_cast<Derived>(base)->called );
  
  // RTTI doesn't provide the derived type when upcast
  EXPECT_NE( &typeid(base), &typeid(derived) );
}