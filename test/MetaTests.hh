#include "gtest/gtest.h"

#include "../util/meta.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

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

  int boolIndex = meta::get_internal<0,bool,bool,int,double,float>::type::index;
  int intIndex = meta::get_internal<0,int,bool,int,double,float>::type::index;
  int doubleIndex = meta::get_internal<0,double,bool,int,double,float>::type::index;
  int floatIndex = meta::get_internal<0,float,bool,int,double,float>::type::index;

  EXPECT_EQ ( 0, boolIndex );
  EXPECT_EQ ( 1, intIndex );
  EXPECT_EQ ( 2, doubleIndex );
  EXPECT_EQ ( 3, floatIndex );

  EXPECT_EQ ( meta::get<bool>(tuple), true );
  EXPECT_EQ ( meta::get<int>(tuple), 1 );
  EXPECT_EQ ( meta::get<double>(tuple), 2.0 );
  EXPECT_EQ ( meta::get<float>(tuple), 3.0f );
}

double sum = 0;

void addToSum(double v)
{
  sum += v;
}

TEST (MetaTest, for_each)
{
  std::tuple<int, double, float> tuple(1, 2.0, 3.0f);
  meta::for_each(tuple, addToSum);

  EXPECT_EQ( sum, 6 );
}
