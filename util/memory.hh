#pragma once

#include <Eigen/Core>
#include <memory>

namespace bold
{
  template<typename T>
  std::shared_ptr<T> allocate_aligned_shared()
  {
    return std::allocate_shared<T>(Eigen::aligned_allocator<T>());
  }
}
