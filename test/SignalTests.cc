#include "gtest/gtest.h"


#include <functional>
#include <sigc++/connection.h>
#include <sigc++/signal.h>

using namespace std;

TEST (SignalTests, nothingConnected)
{
  sigc::signal<void> sig;

  sig();
}

TEST (SignalTests, singleConnected)
{
  sigc::signal<void> sig;

  bool wasCalled = false;

  sig.connect([&] { wasCalled = true; });

  EXPECT_FALSE ( wasCalled );

  sig();

  EXPECT_TRUE ( wasCalled );
}

TEST (SignalTests, multipleConnected)
{
  sigc::signal<void> sig;

  int count = 0;

  sig.connect([&] { count++; });
  sig.connect([&] { count++; });

  sig();

  EXPECT_EQ ( 2, count );
}

TEST (SignalTests, connections)
{
  sigc::signal<void> sig;

  int count = 0;

  sigc::connection conn = sig.connect([&] { count++; });

  sig();

  EXPECT_EQ ( 1, count );

  EXPECT_TRUE ( conn.connected() );

  conn.block();

  sig();

  EXPECT_EQ ( 1, count );
  EXPECT_TRUE ( conn.blocked() );

  conn.unblock();

  EXPECT_EQ ( 1, count );
  EXPECT_FALSE ( conn.blocked() );

  sig();

  EXPECT_EQ ( 2, count );
}
