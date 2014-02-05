#pragma once

#include <Eigen/Core>
#include <memory>
#include <type_traits>

namespace bold
{
  template<typename T, typename... Args>
  std::shared_ptr<T> allocate_aligned_shared(Args... args)
  {
    typedef std::remove_const<T> NonConst;
    return std::allocate_shared<T>(Eigen::aligned_allocator<NonConst>(), args...);
  }
}
