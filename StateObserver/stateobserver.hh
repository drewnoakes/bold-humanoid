#pragma once

#include <memory>

namespace bold
{
  template<typename TState>
  class StateObserver
  {
  public:
    virtual void observe(std::shared_ptr<TState const> state) = 0;
  };
}