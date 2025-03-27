// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_SIGNAL_H
#define _VIA_SIGNAL_H

#include "common.h"
#include <condition_variable>

VIA_NAMESPACE_UTIL_BEGIN

template<typename... Args>
class Signal {
  public:
  using Slot = std::function<void(Args...)>;

  class Connection {
public:
    Connection(std::vector<Slot>& _Slots, std::mutex& _Mutex, size_t _Connection_id)
        : slots(_Slots),
          mutex(_Mutex),
          connection_id(_Connection_id),
          active(true) {}

    void disconnect();

private:
    std::vector<Slot>& slots;
    std::mutex&        mutex;

    size_t connection_id;
    bool   active;
  };

  Connection connect(const Slot&);
  void       fire(Args...);
  void       wait();

  private:
  std::vector<Slot>       slots;
  std::mutex              mutex;
  std::condition_variable condition;
};

VIA_NAMESPACE_END

#endif
