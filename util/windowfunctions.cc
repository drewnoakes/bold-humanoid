#include "windowfunctions.hh"

#include <cmath>

using namespace bold;
using namespace std;

typedef unsigned int uint;

void WindowFunction::create(WindowFunctionType type, vector<double>& window)
{
  switch (type)
  {
    case WindowFunctionType::Rectangular:
      rectangle(window);
      break;
    case WindowFunctionType::Triangular:
      triangular(window);
      break;
    case WindowFunctionType::Welch:
      welch(window);
      break;
    case WindowFunctionType::Hann:
      hann(window);
      break;
    case WindowFunctionType::Hamming:
      hamming(window);
      break;
  }
}

void WindowFunction::rectangle(vector<double>& window)
{
  std::fill(window.begin(), window.end(), 1.0);
}

void WindowFunction::triangular(vector<double>& window)
{
  double a = (window.size() - 1) / 2.0;

  for (uint i = 0; i < window.size(); i++)
    window[i] = 1.0 - fabs(((double)i - a) / a);
}

void WindowFunction::welch(vector<double>& window)
{
  double a = (window.size() - 1) / 2.0;
  double b = (window.size() + 1) / 2.0;

  for (uint i = 0; i < window.size(); i++)
  {
    double c = ((double)i - a) / b;
    window[i] = 1.0 - c*c;
  }
}

void WindowFunction::hann(vector<double>& window)
{
  double a = (2.0 * M_PI) / (window.size() - 1);

  for (uint i = 0; i < window.size(); i++)
    window[i] = 0.5 * (1 - cos(i * a));
}

void WindowFunction::hamming(vector<double>& window)
{
  const double alpha = 0.53836;
  const double beta = 1 - alpha;
  double c = (2.0 * M_PI) / (window.size() - 1);

  for (uint i = 0; i < window.size(); i++)
    window[i] = alpha - beta * cos(i * c);
}
