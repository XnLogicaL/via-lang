// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "signal.h"

VIA_NAMESPACE_UTIL_BEGIN

template<typename... Args>
void Signal<Args...>::Connection::disconnect() {
  if (!active) {
    return;
  }

  std::lock_guard<std::mutex> lock(mutex);
  slots.erase(slots.begin() + connection_id);
}

template<>
void Signal<>::Connection::disconnect() {
  if (!active) {
    return;
  }

  std::lock_guard<std::mutex> lock(mutex);
  slots.erase(slots.begin() + connection_id);
}

template<typename... Args>
Signal<Args...>::Connection Signal<Args...>::connect(const Slot& slot) {
  std::lock_guard<std::mutex> lock(mutex);
  slots.push_back(slot);
  return Connection(slots, mutex, slots.size() - 1);
};

template<>
Signal<>::Connection Signal<>::connect(const Slot& slot) {
  std::lock_guard<std::mutex> lock(mutex);
  slots.push_back(slot);
  return Connection(slots, mutex, slots.size() - 1);
};

template<typename... Args>
void Signal<Args...>::fire(Args... args) {
  std::lock_guard<std::mutex> lock(mutex);

  for (const Slot& slot : slots) {
    slot(args...);
  }

  condition.notify_all();
}

template<>
void Signal<>::fire() {
  std::lock_guard<std::mutex> lock(mutex);

  for (const Slot& slot : slots) {
    slot();
  }

  condition.notify_all();
}

template<typename... Args>
void Signal<Args...>::wait() {
  std::unique_lock<std::mutex> lock(mutex);
  condition.wait(lock);
}

template<>
void Signal<>::wait() {
  std::unique_lock<std::mutex> lock(mutex);
  condition.wait(lock);
}

VIA_NAMESPACE_END
