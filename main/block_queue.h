#pragma once
#include <chrono>
#include <mutex>
#include <queue>
#include <thread>

template <class T> class BlockQueue {
public:
  bool push(const T &val);
  bool pop(T &val);
  int size();

private:
  std::queue<T> _queue;
  std::mutex _mutex;
};

template <class T> int BlockQueue<T>::size() {
  std::lock_guard<std::mutex> lock(_mutex);
  return _queue.size();
}

// TODO add condition
template <class T> bool BlockQueue<T>::push(const T &val) {
  for (int i = 0; i < 10; ++i) {
    {
      std::unique_lock<std::mutex> lock(_mutex);
      if (_queue.size() < 1000) {
        _queue.push(val);
        return true;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return false;
}

template <class T> bool BlockQueue<T>::pop(T &val) {
  for (int i = 0; i < 10; ++i) {
    {
      std::unique_lock<std::mutex> lock(_mutex);
      if (_queue.size() > 0) {
        val = _queue.front();
        _queue.pop();
        return true;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return false;
}
