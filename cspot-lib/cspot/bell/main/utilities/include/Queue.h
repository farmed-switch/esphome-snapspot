#ifndef BELL_QUEUE_H
#define BELL_QUEUE_H

#include <atomic>
#include <condition_variable>
#include <queue>

namespace bell {
template <typename dataType>
class Queue {
 private:

  std::queue<dataType> m_queue;

  mutable std::mutex m_mutex;

  std::condition_variable m_cv;

  std::atomic<bool> m_forceExit = false;

 public:

  void push(dataType const& data) {
    m_forceExit.store(false);
    std::unique_lock<std::mutex> lk(m_mutex);
    m_queue.push(data);
    lk.unlock();
    m_cv.notify_one();
  }

  bool isEmpty() const {
    std::unique_lock<std::mutex> lk(m_mutex);
    return m_queue.empty();
  }

  bool pop(dataType& popped_value) {
    std::unique_lock<std::mutex> lk(m_mutex);
    if (m_queue.empty()) {
      return false;
    } else {
      popped_value = m_queue.front();
      m_queue.pop();
      return true;
    }
  }

  bool wpop(dataType& popped_value) {
    std::unique_lock<std::mutex> lk(m_mutex);
    m_cv.wait(lk,
              [&]() -> bool { return !m_queue.empty() || m_forceExit.load(); });
    if (m_forceExit.load())
      return false;
    popped_value = m_queue.front();
    m_queue.pop();
    return true;
  }

  bool wtpop(dataType& popped_value, long milliseconds = 1000) {
    std::unique_lock<std::mutex> lk(m_mutex);
    m_cv.wait_for(lk, std::chrono::milliseconds(milliseconds), [&]() -> bool {
      return !m_queue.empty() || m_forceExit.load();
    });
    if (m_forceExit.load())
      return false;
    if (m_queue.empty())
      return false;
    popped_value = m_queue.front();
    m_queue.pop();
    return true;
  }

  int size() {
    std::unique_lock<std::mutex> lk(m_mutex);
    return static_cast<int>(m_queue.size());
  }

  void clear() {
    m_forceExit.store(true);
    std::unique_lock<std::mutex> lk(m_mutex);
    while (!m_queue.empty()) {

      m_queue.pop();
    }
    lk.unlock();
    m_cv.notify_one();
  }

  bool isExit() const { return m_forceExit.load(); }
};
}

#endif