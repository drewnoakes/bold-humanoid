#pragma once

#include <cassert>
#include <memory>

namespace bold
{
  class StateObject;

  class StateObserver
  {
  public:
    virtual void observe(std::shared_ptr<StateObject const> state) = 0;
  };

  template<typename TState>
  class TypedStateObserver : public StateObserver
  {
  public:
    void observe(std::shared_ptr<StateObject const> state) override
    {
      std::shared_ptr<TState const> typedState = std::dynamic_pointer_cast<TState const>(state);
      assert(typedState);
      observeTyped(typedState);
    }

    virtual void observeTyped(std::shared_ptr<TState const> state) = 0;
  };
}
