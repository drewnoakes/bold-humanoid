#pragma once

#include <algorithm>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <sigc++/signal.h>
#include <type_traits>
#include <typeindex>
#include <vector>

#include "../SequentialTimer/sequentialtimer.hh"
#include "../StateObject/stateobject.hh"
#include "../StateObserver/stateobserver.hh"
#include "../util/assert.hh"
#include "../util/log.hh"
#include "../util/memory.hh"

struct libwebsocket_protocols;

namespace bold
{
  enum class StateTime
  {
    MostRecent,
    CameraImage
  };

  class StateTracker
  {
  public:
    template<typename T>
    static std::shared_ptr<StateTracker> create(std::string name)
    {
      static_assert(std::is_base_of<StateObject, T>::value, "T must be a descendant of StateObject");
      return std::make_shared<StateTracker>(name);
    }

    StateTracker(std::string name)
    : websocketProtocol(nullptr),
      d_name(name)
    {}

    void set(std::shared_ptr<StateObject const> state)
    {
      std::lock_guard<std::mutex> guard(d_mutex);
      d_stateMostRecent = state;
      d_updateCount++;
    }

    void snapshot(StateTime time)
    {
      if (time == StateTime::CameraImage)
      {
        std::lock_guard<std::mutex> guard(d_mutex);
        d_stateCameraImage = d_stateMostRecent;
      }
      else
      {
        log::error("StateTracker::snapshot") << "Unexpected StateTime value: " << (int)time;
        throw std::runtime_error("Unexpected StateTime value");
      }
    }

    template<typename T>
    std::shared_ptr<T const> state(StateTime time = StateTime::MostRecent) const
    {
      static_assert(std::is_base_of<StateObject, T>::value, "T must be a descendant of StateObject");
      std::lock_guard<std::mutex> guard(d_mutex);
      return std::dynamic_pointer_cast<T const>(time == StateTime::MostRecent ? d_stateMostRecent : d_stateCameraImage);
    }

    std::shared_ptr<StateObject const> stateBase(StateTime time = StateTime::MostRecent) const
    {
      std::lock_guard<std::mutex> guard(d_mutex);
      return time == StateTime::MostRecent ? d_stateMostRecent : d_stateCameraImage;
    }

    std::string name() const { return d_name; }
    long long unsigned updateCount() const { return d_updateCount; }

    libwebsocket_protocols* websocketProtocol;

  private:
    mutable std::mutex d_mutex;
    const std::string d_name;
    std::shared_ptr<StateObject const> d_stateMostRecent;
    std::shared_ptr<StateObject const> d_stateCameraImage;
    long long unsigned d_updateCount;
  };

  class State
  {
  public:
    static void initialise();

    template<typename T>
    static void registerStateType(std::string name);

    static std::vector<std::shared_ptr<StateTracker>> getTrackers();

    static unsigned stateTypeCount() { return d_trackerByTypeId.size(); }

    /** Fires when a state object is updated.
     *
     * Currently only used by DataStreamer, which may serialise the
     * StateObject for attached clients.
     *
     * A lock is held over d_mutex while this event is raised :(
     *
     * TODO avoid unbounded holding of d_mutex by getting rid of this
     * signal and using a queue for the DataStreamer thread to process
     * (?)
     */
    static sigc::signal<void, std::shared_ptr<StateTracker const>> updated;

    static void registerObserver(std::shared_ptr<StateObserver> observer);

    static void callbackObservers(ThreadId threadId, SequentialTimer& timer);

    template <typename T>
    static void set(std::shared_ptr<T const> state);

    template<typename T, typename... TArgs>
    static std::shared_ptr<T const> make(TArgs&&... args);

    /** Get the StateObject of specified type T. May be nullptr.
     */
    template <typename T>
    static std::shared_ptr<T const> get(StateTime time = StateTime::MostRecent);

    static std::shared_ptr<StateObject const> getByName(std::string name);

    template<typename T>
    static std::shared_ptr<T const> getTrackerState(StateTime time = StateTime::MostRecent);

    static std::shared_ptr<StateObject const> getByTypeIndex(std::type_index const& typeIndex);

    template<typename T>
    static std::shared_ptr<StateTracker> getTracker();

    static void snapshot(StateTime time)
    {
      std::lock_guard<std::mutex> guard(d_mutex);
      for (auto const& pair : d_trackerByTypeId)
        pair.second->snapshot(time);
    }

  private:
    State() = delete;

    static std::mutex d_mutex;

    static std::unordered_map<std::type_index, std::vector<std::shared_ptr<StateObserver>>> d_observersByTypeIndex;
    static std::unordered_map<int, std::vector<std::shared_ptr<StateObserver>>> d_observersByThreadId;
    static std::unordered_map<std::type_index, std::shared_ptr<StateTracker>> d_trackerByTypeId;
    static std::unordered_map<std::string,     std::shared_ptr<StateTracker>> d_trackerByName;
  };

  template<typename T>
  void State::registerStateType(std::string name)
  {
    static_assert(std::is_base_of<StateObject, T>::value, "T must be a descendant of StateObject");
    log::verbose("State::registerStateType") << "Registering state type: " << name;

    std::lock_guard<std::mutex> guard(d_mutex);
    if (d_trackerByTypeId.find(typeid(T)) != d_trackerByTypeId.end())
    {
      log::warning("State::registerStateType") << "State type already registered: " << name;
      return;
    }

    auto const& tracker = StateTracker::create<T>(name);
    d_trackerByTypeId[typeid(T)] = tracker;
    d_trackerByName[name] = tracker;

    // Create an empty vector for the observers so that we don't have to lock later on the observer map
    d_observersByTypeIndex[typeid(T)] = {};
  }

  template <typename T>
  void State::set(std::shared_ptr<T const> state)
  {
    static_assert(std::is_base_of<StateObject, T>::value, "T must be a descendant of StateObject");

    auto const& tracker = getTracker<T const>();
    tracker->set(state);

    std::vector<std::shared_ptr<StateObserver>>* observers;

    // Raise event and look up observers while holding lock
    {
      // TODO this blocks for too long. eventing won't work well. need to do all updates async, off the motion thread
      std::lock_guard<std::mutex> guard(d_mutex);
      updated(tracker);

      auto it = d_observersByTypeIndex.find(typeid(T));
      ASSERT(it != d_observersByTypeIndex.end());
      observers = &it->second;
    }

    // Release lock before notifying observers
    for (auto& observer : *observers)
    {
      ASSERT(observer);
      observer->setDirty();
    }
  }

  template<typename T, typename... Args>
  std::shared_ptr<T const> State::make(Args&&... args)
  {
    std::shared_ptr<T const> state = allocate_aligned_shared<T const>(std::forward<Args>(args)...);
    set(state);
    return state;
  }

  template <typename T>
  std::shared_ptr<T const> State::get(StateTime time)
  {
    static_assert(std::is_base_of<StateObject, T>::value, "T must be a descendant of StateObject");
    return State::getTrackerState<T>(time);
  }

  template<typename T>
  std::shared_ptr<T const> State::getTrackerState(StateTime time)
  {
    static_assert(std::is_base_of<StateObject, T>::value, "T must be a descendant of StateObject");
    std::lock_guard<std::mutex> guard(d_mutex);
    auto pair = d_trackerByTypeId.find(typeid(T));
    ASSERT(pair != d_trackerByTypeId.end() && "Tracker type must be registered");
    return pair->second->state<T>(time);
  }

  template<typename T>
  std::shared_ptr<StateTracker> State::getTracker()
  {
    static_assert(std::is_base_of<StateObject, T>::value, "T must be a descendant of StateObject");
    std::lock_guard<std::mutex> guard(d_mutex);
    auto pair = d_trackerByTypeId.find(typeid(T));
    ASSERT(pair != d_trackerByTypeId.end() && "Tracker type must be registered");
    auto tracker = pair->second;
    return tracker;
  }
}
