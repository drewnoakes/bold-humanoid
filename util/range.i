%{
#include <util/Range.hh>
%}

namespace bold
{
  template<typename T>
  class Range
  {
  public:
    Range();
    Range(T min, T max);

    bool isEmpty() const;
    T min() const;
    T max() const;

    void expand(T value);

    void expand(Range<T> value);

    void reset();

    T size() const;

    bool contains(T value) const;

    T clamp(T value) const;

    bool operator==(Range<T> const& other) const;

    bool operator!=(Range<T> const& other) const;
  };

  %template(IntRange) Range<int>;
  %template(DblRange) Range<double>;
}
