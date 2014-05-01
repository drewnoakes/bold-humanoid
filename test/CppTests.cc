#include "gtest/gtest.h"


#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <algorithm>

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
  // See http://en.wikipedia.org/wiki/Erase-remove_idiom

  vector<int> vec = {1, 2, 3, 3, 4, 3};

  auto it1 = remove_if(
    vec.begin(), vec.end(),
    [](int i) { return i == 3;}
  );

  EXPECT_EQ( 6, vec.size() );
  EXPECT_EQ( 1, vec[0] );
  EXPECT_EQ( 2, vec[1] );
  EXPECT_EQ( 4, vec[2] ); // NOTE 4 moved forwards
  EXPECT_EQ( 3, vec[3] );
  EXPECT_EQ( 4, vec[4] ); // NOTE 4 still exists in this position
  EXPECT_EQ( 3, vec[5] );

  EXPECT_EQ ( &(*it1), vec.data() + 3 ); // points to start of data to remove

  auto it2 = vec.erase(it1, vec.end());

  EXPECT_EQ( vec.end(), it2 );

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

TEST (CppTests, vectorEraseSharedPtr)
{
  for (int thres = 0; thres < 4; thres++)
  {
    vector<shared_ptr<int>> ptrs;

    ptrs.push_back(make_shared<int>(0));
    ptrs.push_back(make_shared<int>(1));
    ptrs.push_back(make_shared<int>(2));
    ptrs.push_back(make_shared<int>(3));

    ptrs.erase(
      remove_if(
        ptrs.begin(), ptrs.end(),
        [thres](shared_ptr<int> i) { EXPECT_TRUE(bool(i)); return *i >= thres; }
      ),
      ptrs.end()
    );

    ASSERT_TRUE( std::all_of(ptrs.begin(), ptrs.end(), [](shared_ptr<int> p) { return bool(p); }));

    EXPECT_EQ( thres, ptrs.size() );
  }
}

TEST(CppTests, mutateVectorInMap)
{
  map<int,vector<int>> data;

  data[1] = {1,1,1,1,1};
  data[2] = {2,2,2,2,2};

  auto& ones = data[1];

  ones[0] = 2;

  EXPECT_EQ(2, data[1][0]);

  auto& retrieved = data[1];

  retrieved[1] = 3;

  EXPECT_EQ(3, data[1][1]);
}

TEST(CppTests, std_function)
{
  function<bool()> fun = nullptr;

  ASSERT_TRUE ( fun == nullptr );
  ASSERT_TRUE ( !fun );
  ASSERT_FALSE ( fun );
}

TEST(CppTests, setsAndSharedPtr)
{
  auto p1 = make_shared<int>(1);
  auto p2 = make_shared<int>(1);
  auto p1copy = p1;

  set<shared_ptr<int>> s;
  s.insert(p1);

  EXPECT_TRUE(s.find(p1) != s.end());
  EXPECT_TRUE(s.find(p1copy) != s.end());
  EXPECT_TRUE(s.find(p2) == s.end());
}

TEST(CppTests, chrono_duration)
{
  EXPECT_TRUE( chrono::seconds(2) > chrono::seconds(1) );
  EXPECT_TRUE( chrono::milliseconds(1001) > chrono::seconds(1) );
}

///
/// ATOMIC OPERATIONS
///
/// See http://en.cppreference.com/w/cpp/memory/shared_ptr/atomic
///

