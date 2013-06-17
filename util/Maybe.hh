#pragma once

#include <memory>
#include <ostream>

namespace bold
{
  template<typename T>
  struct Maybe : public std::shared_ptr<T>
  {
    bool hasValue() const { return this->get(); }
    T const& value() const { return *(this->get()); }

    static Maybe<T> empty() { return Maybe<T>(0); }

    Maybe(T value)
      : std::shared_ptr<T>(std::make_shared<T>(value))
    {}

    Maybe(std::shared_ptr<T> value)
      : std::shared_ptr<T>(value)
    {}

    bool operator==(Maybe const& other) const
    {
      if (hasValue() != other.hasValue())
        return false;
      return !hasValue() || *(this->get()) == *(other.get());
    }

    friend std::ostream& operator<<(std::ostream& stream, Maybe<T> const& maybe)
    {
      return maybe
        ? stream << "Maybe (hasValue=true value=" << *maybe << ")"
        : stream << "Maybe (hasValue=false)";
    }
  };
}
