// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "signal.h"

namespace via::utils {

template<typename... Args>
void signal<Args...>::sig_connection_t::disconnect() {
  if (!active) {
    return;
  }

  std::lock_guard<std::mutex> lock(mutex);
  slots.erase(slots.begin() + connection_id);
}

template<>
void signal<>::sig_connection_t::disconnect() {
  if (!active) {
    return;
  }

  std::lock_guard<std::mutex> lock(mutex);
  slots.erase(slots.begin() + connection_id);
}

template<typename... Args>
signal<Args...>::sig_connection_t signal<Args...>::connect(const sig_slot_t& slot) {
  std::lock_guard<std::mutex> lock(mutex);
  slots.push_back(slot);
  return sig_connection_t(slots, mutex, slots.size() - 1);
};

template<>
signal<>::sig_connection_t signal<>::connect(const sig_slot_t& slot) {
  std::lock_guard<std::mutex> lock(mutex);
  slots.push_back(slot);
  return sig_connection_t(slots, mutex, slots.size() - 1);
};

template<typename... Args>
void signal<Args...>::fire(Args... args) {
  std::lock_guard<std::mutex> lock(mutex);

  for (const sig_slot_t& slot : slots) {
    slot(args...);
  }

  condition.notify_all();
}

template<>
void signal<>::fire() {
  std::lock_guard<std::mutex> lock(mutex);

  for (const sig_slot_t& slot : slots) {
    slot();
  }

  condition.notify_all();
}

template<typename... Args>
void signal<Args...>::wait() {
  std::unique_lock<std::mutex> lock(mutex);
  condition.wait(lock);
}

template<>
void signal<>::wait() {
  std::unique_lock<std::mutex> lock(mutex);
  condition.wait(lock);
}

} // namespace via::utils
