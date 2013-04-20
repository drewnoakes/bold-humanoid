#ifndef BOLD_LAZY_HH
#define BOLD_LAZY_HH

#include <memory>

#include "Maybe.hh"

namespace bold
{
  template<typename T>
  struct Lazy
  {
    Lazy(std::function<std::shared_ptr<T>()> creator)
    : d_value(Maybe<T>::empty()),
      d_creator(creator)
    {}

    Lazy()
    : d_value(Maybe<T>::empty()),
      d_creator(nullptr)
    {}

    bool hasValue() const { return d_value.hasValue(); }

    std::shared_ptr<T> value() const
    {
      if (hasValue())
        return d_value.value();

      if (d_creator)
      {
        auto writeable = const_cast<Maybe<T>*>(&d_value);
        *writeable = Maybe<T>(d_creator());
        return d_value.value();
      }

      return nullptr;
    }

    operator bool() const
    {
      return hasValue();
    }

    T const* operator->() const
    {
      return value();
    }

    T const* operator*() const
    {
      return value();
    }

//     bool operator==(Maybe const& other) const
//     {
//       if (d_hasValue ^ other.d_hasValue)
//         return false;
//
//       return !d_hasValue || *d_value == *other.d_value;
//     }
//
//     friend std::ostream& operator<<(std::ostream& stream, Maybe<T> const& maybe)
//     {
//       if (maybe.d_hasValue)
//       {
//         return stream << "Maybe (hasValue=true value=" << *maybe.d_value << ")";
//       }
//       return stream << "Maybe (hasValue=false)";
//     }

  private:
    Maybe<T> d_value;
    std::function<std::shared_ptr<T>()> d_creator;
  };
}

#endif
