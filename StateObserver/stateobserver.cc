#include "stateobserver.hh"

using namespace bold;
using namespace std;

StateObserver::StateObserver(string observerName, ThreadId callbackThread)
: d_types(),
  d_callbackThreadId(callbackThread),
  d_name(observerName)
{
  // Initially, not dirty
  d_lock.test_and_set(memory_order_acquire);
}

void StateObserver::setDirty() { d_lock.clear(memory_order_release); }

bool StateObserver::testAndClearDirty()
{
  ASSERT(ThreadUtil::getThreadId() == d_callbackThreadId);

  // Prevent race condition
  // If flag is false, then the state is dirty
  return !d_lock.test_and_set(memory_order_acquire);
}
