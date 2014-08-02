#pragma once

#include <vector>

namespace bold
{
  enum class WindowFunctionType
  {
    Rectangular = 1,
    Triangular = 2,
    Welch = 3,
    Hann = 4,
    Hamming = 5
  };

  class WindowFunction
  {
  public:
    static void create(WindowFunctionType type, std::vector<double>& window);

    static void rectangle(std::vector<double>& window);
    static void triangular(std::vector<double>& window);
    static void welch(std::vector<double>& window);
    static void hann(std::vector<double>& window);
    static void hamming(std::vector<double>& window);

  private:
    WindowFunction() = delete;
  };
}
