#include <memory>
#include <mutex>

#include "../include/configure.hpp"
#include "../include/current_calls.hpp"
#include "../include/notify_interface.hpp"

using s_ptr = std::shared_ptr<session>;

current_calls::current_calls() : ni_ptr(&ni) {}

std::size_t current_calls::join(s_ptr se) {
  std::lock_guard<std::mutex> Lk{Mut};
  for (std::size_t i = 0; i < NCalls; ++i) {
    if (sessions_.find(i) == sessions_.end()) {
      sessions_.insert({i, se});
      if (size() >= NCalls)
        ni_ptr->done = false;
      return i;
    }
  }
  return -1;
}

s_ptr current_calls::operator[](std::size_t opid) { return sessions_[opid]; }

void current_calls::leave(std::size_t opid) {
  std::lock_guard<std::mutex> Lk{ni_ptr->Mut};
  sessions_.erase(opid);
  ni_ptr->done = true;
  ni_ptr->Cond.notify_one();
}

std::size_t current_calls::size() { return sessions_.size(); }
