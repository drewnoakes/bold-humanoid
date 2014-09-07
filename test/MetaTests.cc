#include "gtest/gtest.h"

#include "../util/meta.hh"

using namespace std;
using namespace bold;

TEST (MetaTests, if_)
{
  int trueVal = meta::if_<true, 1, 2>::value; 
  int falseVal = meta::if_<false, 1, 2>::value; 

  EXPECT_EQ( 1, trueVal );
  EXPECT_EQ( 2, falseVal );
}

TEST (MetaTests, min_)
{
  int minVal1 = meta::min<1, 2>::value;
  int minVal2 = meta::min<2, 1>::value;

  EXPECT_EQ( 1, minVal1 );
  EXPECT_EQ( 1, minVal2 );
}

TEST (MetaTests, get)
{
  std::tuple<bool, int, double, float> tuple(true, 1, 2.0, 3.0f);

#if __cplusplus <= 201103L
  int boolIndex = meta::get_internal<0,bool,bool,int,double,float>::type::index;
  int intIndex = meta::get_internal<0,int,bool,int,double,float>::type::index;
  int doubleIndex = meta::get_internal<0,double,bool,int,double,float>::type::index;
  int floatIndex = meta::get_internal<0,float,bool,int,double,float>::type::index;

  EXPECT_EQ ( 0, boolIndex );
  EXPECT_EQ ( 1, intIndex );
  EXPECT_EQ ( 2, doubleIndex );
  EXPECT_EQ ( 3, floatIndex );
#endif

  EXPECT_EQ ( meta::get<bool>(tuple), true );
  EXPECT_EQ ( meta::get<int>(tuple), 1 );
  EXPECT_EQ ( meta::get<double>(tuple), 2.0 );
  EXPECT_EQ ( meta::get<float>(tuple), 3.0f );
}

double sum = 0;

struct AddToSum
{
  static void do_it(double v)
  {
    sum += v;
  }
};

struct MultAndSum
{
  static void do_it(double v, double factor)
  {
    sum += v * factor;
  }
};

struct TemplFun
{
  template<typename T>
  static void do_it(T v)
  {
    sum += 1;
  }

  static void do_it(int v)
  {
    sum += 10;
  }
};

TEST (MetaTests, for_each)
{
  // Test unary function
  sum = 0;
  std::tuple<int, double, float> tuple(1, 2.0, 3.0f);
  meta::for_each<AddToSum>(tuple);
  EXPECT_EQ( sum, 6 );

  // Test multiple arguments
  sum = 0;
  meta::for_each<MultAndSum>(tuple, 2.0);
  EXPECT_EQ( sum, 12 );

  // Test template function
  sum = 0;
  meta::for_each<TemplFun>(tuple);
  EXPECT_EQ( sum, 12 );
}
