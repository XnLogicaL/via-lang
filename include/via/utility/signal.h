// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _vl_signal_h
#define _vl_signal_h

#include "common.h"
#include <condition_variable>

namespace via::utils {

template<typename... Args>
class signal {
public:
  using sig_slot_t = std::function<void(Args...)>;

  class sig_connection_t {
  public:
    sig_connection_t(std::vector<sig_slot_t>& _Slots, std::mutex& _Mutex, size_t _Connection_id)
      : slots(_Slots),
        mutex(_Mutex),
        connection_id(_Connection_id),
        active(true) {}

    void disconnect();

  private:
    std::vector<sig_slot_t>& slots;
    std::mutex& mutex;

    size_t connection_id;
    bool active;
  };

  sig_connection_t connect(const sig_slot_t&);
  void fire(Args...);
  void wait();

private:
  std::vector<sig_slot_t> slots;
  std::mutex mutex;
  std::condition_variable condition;
};

} // namespace via::utils

#endif
