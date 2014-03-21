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
    typedef typename std::remove_const<T>::type NonConst;
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

#if __cplusplus == 201103L
// Stub for std::make_unique which will be added in c++14
namespace std
{
  template<typename T, typename... Args>
  std::unique_ptr<T> make_unique(Args&&... args)
  {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }
}
#endif
