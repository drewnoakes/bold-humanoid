#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "log.hh"

namespace bold
{
  template<typename T>
  class ConsumerQueueThread
  {
  public:
    ConsumerQueueThread(std::function<void(T)> processor)
    : d_processor(processor),
      d_thread(std::thread(&ConsumerQueueThread::run, this)),
      d_stop(false)
    {}

    ConsumerQueueThread(ConsumerQueueThread const&) = delete;
    ConsumerQueueThread& operator=(ConsumerQueueThread const&) = delete;

    void push(const T& item)
    {
      if (d_stop)
      {
        log::error("ConsumerQueueThread::push") << "Invalid operation once stopped";
        throw std::runtime_error("Invalid operation once stopped");
      }

      std::unique_lock<std::mutex> lock(d_mutex);
      d_queue.push(item);
      lock.unlock();
      d_condition.notify_one();
    }

    void stop()
    {
      std::unique_lock<std::mutex> lock(d_mutex);
      d_stop = true;
      lock.unlock();
      d_condition.notify_one();
      d_thread.join();
    }

  private:
    void run()
    {
      while (true)
      {
        std::unique_lock<std::mutex> lock(d_mutex);

        // Wait for an item to appear in the queue.
        // Note that we may loop around due to 'spurious wakes'.
        // Check that stop hasn't been requested as well.
        while (!d_stop && d_queue.empty())
        {
          d_condition.wait(lock);
        }

        if (d_stop)
          return;

        T item = d_queue.front();
        d_queue.pop();
        d_processor(item);
      }
    }

    std::queue<T> d_queue;
    std::mutex d_mutex;
    std::condition_variable d_condition;
    std::function<void(T)> d_processor;
    std::thread d_thread;
    bool d_stop;
  };
}
