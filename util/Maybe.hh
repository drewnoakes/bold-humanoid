#ifndef BOLD_MAYBE_HH
#define BOLD_MAYBE_HH

#include <memory>

namespace bold
{
  template<typename T>
  struct Maybe
  {
    bool hasValue() { return d_hasValue; }
    std::shared_ptr<T> value() { return d_value; }

    static Maybe<T> empty() { return Maybe<T>(false); }

    Maybe(T value)
    : d_hasValue(true),
      d_value(std::make_shared<T>(value))
    {}

    Maybe(std::shared_ptr<T> value)
    : d_hasValue(true),
      d_value(value)
    {}

    bool operator==(Maybe const& other) const
    {
      if (d_hasValue ^ other.d_hasValue)
        return false;

      return !d_hasValue || *d_value == *other.d_value;
    }

    friend std::ostream& operator<<(std::ostream& stream, Maybe<T> const& maybe)
    {
      if (maybe.d_hasValue)
      {
        return stream << "Maybe (hasValue=true value=" << *maybe.d_value << ")";
      }
      return stream << "Maybe (hasValue=false)";
    }

  private:
    Maybe(bool hasValue)
    : d_hasValue(false),
      d_value(0)
    {}

    bool d_hasValue;
    std::shared_ptr<T> d_value;
  };
}

#endif