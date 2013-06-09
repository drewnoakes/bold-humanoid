#include "gtest/gtest.h"

#include "helpers.hh"

#include <memory>

using namespace std;

class Base
{
public:
  virtual ~Base() {}
  virtual bool isSomething() = 0;
  virtual bool isSub() { return false; }
};

class Derived : public Base
{
public:
  bool called;
  bool isSomething() override { called = true; return true; }
  bool isSub() override { return true; }
};

TEST (CppTests, dynamicSharedPointerCast)
{
  shared_ptr<Derived> sub = make_shared<Derived>();
  shared_ptr<Base> base = sub;

  shared_ptr<Derived> sub2 = dynamic_pointer_cast<Derived>(base);
      
  EXPECT_TRUE ( sub->isSub() );
  EXPECT_TRUE ( base->isSub() );
  EXPECT_TRUE ( sub2->isSub() );
}

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

TEST (CppTests, reassigningSharedPointers)
{
  auto a = make_shared<int>(1);
  
  EXPECT_EQ( 1, *a );
  
  auto b = a;
  
  EXPECT_EQ( 1, *a );
  EXPECT_EQ( 1, *b );
  
  // Reassigning the object pointed to affects all copies of the shared_ptr
  *b = 2;
  
  EXPECT_EQ( 2, *a );
  EXPECT_EQ( 2, *b );
  
  // Reassigning an instance of shared_ptr with another shared_ptr affects only the copy be assigned to
  a = make_shared<int>(3);

  EXPECT_EQ( 3, *a );
  EXPECT_EQ( 2, *b );
}

TEST (CppTests, resettingSharedPointers)
{
  auto a = make_shared<int>(1);
  
  EXPECT_EQ( 1, *a );
  
  auto b = a;
  
  EXPECT_EQ( a, b );
  EXPECT_EQ( 1, *a );
  EXPECT_EQ( 1, *b );
 
  a.reset(new int);
  *a = 2;
  
  EXPECT_EQ( 2, *a );
  EXPECT_EQ( 1, *b );
 
  a.reset();

  EXPECT_EQ( nullptr, a );
  EXPECT_EQ( 1, *b );
}

TEST (CppTests, referenceCounting)
{
  auto a = make_shared<int>(1);
  
  EXPECT_EQ( 1, a.use_count() );
  EXPECT_TRUE( a.unique() );
  
  auto b = a;
  
  EXPECT_EQ( 2, a.use_count() );
  EXPECT_EQ( 2, b.use_count() );
  EXPECT_FALSE( a.unique() );
  EXPECT_FALSE( b.unique() );
 
  a.reset();
  
  EXPECT_EQ( 1, b.use_count() );
  EXPECT_TRUE( b.unique() );
}

TEST (CppTests, swappingSharedPointers)
{
  auto a = make_shared<int>(1);
  auto b = make_shared<int>(2);

  EXPECT_EQ( 1, *a );
  EXPECT_EQ( 2, *b );

  b.swap(a);
  
  EXPECT_EQ( 2, *a );
  EXPECT_EQ( 1, *b );
}

TEST (CppTests, vectorEraseRemove)
{
  vector<int> vec = {1, 2, 3, 3, 3, 4};
  
  auto it = vec.erase(
    remove_if(
      vec.begin(), vec.end(),
      [](int i) { return i == 3;}
    ),
    vec.end()
  );
  
  EXPECT_EQ( 3, *it );
  
  EXPECT_EQ( 3, vec.size() );
  EXPECT_EQ( 1, vec[0] );
  EXPECT_EQ( 2, vec[1] );
  EXPECT_EQ( 4, vec[2] );
}

TEST (CppTests, vectorEraseRemoveNonConsecutive)
{
  vector<int> vec = {1, 2, 3, 4, 5, 6};
  
  auto it = vec.erase(
    remove_if(
      vec.begin(), vec.end(),
      [](int i) { return i%2 == 0;}
    ),
    vec.end()
  );
  
  EXPECT_EQ( 4, *it );
  
  EXPECT_EQ( 3, vec.size() );
  EXPECT_EQ( 1, vec[0] );
  EXPECT_EQ( 3, vec[1] );
  EXPECT_EQ( 5, vec[2] );
}

TEST (CppTests, vectorEraseRemoveNothingMatched)
{
  vector<int> vec = {1, 2, 3, 4, 5, 6};
  vector<int> seen;
  
  auto it = vec.erase(
    remove_if(
      vec.begin(), vec.end(),
      [&seen](int i) {
        seen.push_back(i);
        return false;
      }
    ),
    vec.end()
  );
  
  EXPECT_EQ( vec.end(), it );
  
  EXPECT_EQ( 6, vec.size() );
  EXPECT_EQ( 6, seen.size() );
  EXPECT_EQ( 1, seen[0] );
  EXPECT_EQ( 2, seen[1] );
  EXPECT_EQ( 3, seen[2] );
  EXPECT_EQ( 4, seen[3] );
  EXPECT_EQ( 5, seen[4] );
  EXPECT_EQ( 6, seen[5] );
}

TEST (CppTests, vectorEraseWhenNoElements)
{
  vector<int> vec;
  vector<int> seen;
  
  auto it = vec.erase(
    remove_if(
      vec.begin(), vec.end(),
      [&seen](int i) {
        seen.push_back(i);
        return false;
      }
    ),
    vec.end()
  );
  
  EXPECT_EQ( vec.end(), it );
  
  EXPECT_EQ( 0, vec.size() );
  EXPECT_EQ( 0, seen.size() );
}

TEST (CppTests, vectorEraseRemoveWhenEmpty)
{
  vector<int> vec;
  
  vec.erase(
    remove_if(
      vec.begin(), vec.end(),
      [](int i) { return i == 3;}
    ),
    vec.end()
  );
  
  EXPECT_EQ( 0, vec.size() );
}

///
/// ATOMIC OPERATIONS
///
/// See http://en.cppreference.com/w/cpp/memory/shared_ptr/atomic
///

