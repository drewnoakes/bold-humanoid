#pragma once

#include <atomic>
#include <cassert>
#include <memory>
#include <vector>

#include "stateobserver.hh"

#include "../State/state.hh"
#include "../SequentialTimer/sequentialtimer.hh"

namespace bold
{
  class StateObject;

  //
  // TypedStateObserver class template
  //

  template<typename TState>
  class TypedStateObserver : public StateObserver
  {
  public:
    TypedStateObserver(std::string observerName, ThreadId callbackThread)
    : StateObserver(observerName, callbackThread),
      d_typeIndex(typeid(TState))
    {
      static_assert(std::is_base_of<StateObject, TState>::value, "T must be a descendant of StateObject");
      d_types.push_back(d_typeIndex);
    }

    void observe(SequentialTimer& timer) override
    {
      assert(ThreadUtil::getThreadId() == d_callbackThreadId);
      std::shared_ptr<StateObject const> state = State::getByTypeIndex(d_typeIndex);
      std::shared_ptr<TState const> typedState = std::dynamic_pointer_cast<TState const>(state);
      assert(typedState);
      observeTyped(typedState, timer);
    }

    virtual void observeTyped(std::shared_ptr<TState const> const& state, SequentialTimer& timer) = 0;

  private:
    std::type_index d_typeIndex;
  };
}
