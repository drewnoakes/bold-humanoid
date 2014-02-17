#pragma once

#include <Eigen/Core>
#include <memory>
#include <type_traits>
#include <utility>

namespace bold
{
#if EIGEN_ALIGN
  template<typename T, typename... Args>
  std::shared_ptr<T> allocate_aligned_shared(Args&&... args)
  {
    typedef std::remove_const<T> NonConst;
    return std::allocate_shared<T>(Eigen::aligned_allocator<NonConst>(), std::forward<Args>(args)...);
  }
#else
  template<typename T, typename... Args>
  std::shared_ptr<T> allocate_aligned_shared(Args&&... args)
  {
    return std::make_shared<T>(std::forward<Args>(args)...);
  }
#endif
}
