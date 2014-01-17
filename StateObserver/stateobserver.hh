#pragma once

#include <atomic>
#include <cassert>
#include <memory>
#include <vector>
#include <typeindex>

#include "../ThreadUtil/threadutil.hh"

namespace bold
{
  class SequentialTimer;

  //
  // StateObserver class
  //

  class StateObserver
  {
  public:
    StateObserver(std::string observerName, ThreadId callbackThread)
    : d_types(),
      d_callbackThreadId(callbackThread),
      d_name(observerName)
    {}

    virtual ~StateObserver() {}

    virtual void observe(SequentialTimer& timer) = 0;

    void setDirty();

    bool testAndClearDirty();

    std::string getName() const { return d_name; }
    std::vector<std::type_index> const& getTypes() const { return d_types; }
    ThreadId getCallbackThreadId() const { return d_callbackThreadId; }

  protected:
    std::vector<std::type_index> d_types;
    const ThreadId d_callbackThreadId;
    const std::string d_name;

  private:
    std::atomic_flag d_lock = ATOMIC_FLAG_INIT;
  };
}
