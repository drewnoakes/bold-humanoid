#pragma once

#include <memory>

#include "../Clock/clock.hh"

namespace bold
{
  auto negate = [](std::function<bool()> condition)
  {
    return [condition]() { return !condition(); };
  };

  auto isRepeated = [](int times, std::function<bool()> condition)
  {
    auto count = std::make_shared<int>(0);
    return [count,times,condition]()
    {
      if (condition())
      {
        (*count)++;
        return *count >= times;
      }
      *count = 0;
      return false;
    };
  };

  auto trueNTimes = [](int times)
  {
    auto count = std::make_shared<int>(0);
    return [count,times]()
    {
      (*count)++;
      return *count <= times;
    };
  };

  auto trueForMillis = [](int millis, std::function<bool()> condition)
  {
    auto turnedTrueAt = std::make_shared<double>(0);
    auto lastTrue = std::make_shared<bool>(false);

    return [turnedTrueAt,lastTrue,millis,condition]()
    {
      if (condition())
      {
        auto t = Clock::getMillis();
        if (*lastTrue)
        {
          if (t - *turnedTrueAt > millis)
          {
            return true;
          }
        }
        else
        {
          *lastTrue = true;
          *turnedTrueAt = t;
        }
      }
      else
      {
        *lastTrue = false;
      }
      return false;
    };
  };

  auto stepUpDownThreshold = [](int threshold, std::function<bool()> condition)
  {
    auto count = std::make_shared<int>(0);

    return [count,condition,threshold]()
    {
      if (condition())
      {
        if (*count < threshold)
        {
          (*count)++;
          return *count == threshold;
        }

        return true;
      }
      else if (*count > 0)
      {
        (*count)--;
      }

      return false;
    };
  };

  auto oneShot = [](std::function<std::function<bool()>()> factory)
  {
    auto currentFun = make_shared<std::function<bool()>>();
    auto hasFun = std::make_shared<bool>(false);

    return [currentFun,factory,hasFun]()
    {
      if (!(*hasFun))
      {
        *currentFun = factory();
        *hasFun = true;
      }

      if ((*currentFun)())
      {
        *currentFun = nullptr;
        *hasFun = false;
        return true;
      }

      return false;
    };
  };
}
