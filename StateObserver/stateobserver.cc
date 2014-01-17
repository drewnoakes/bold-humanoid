#include "stateobserver.hh"

using namespace bold;
using namespace std;

void StateObserver::setDirty() { d_lock.clear(memory_order_release); }

bool StateObserver::testAndClearDirty()
{
  assert(ThreadUtil::getThreadId() == d_callbackThreadId);
  
  // Prevent race condition
  // If flag is false, then the state is dirty
  return !d_lock.test_and_set(memory_order_acquire);
}
