#ifndef CURRENT_CALLS_HPP
#define CURRENT_CALLS_HPP

#include <cstddef>
#include <mutex>
#include <unordered_map>

#include "configure.hpp"
#include "notify_interface.hpp"

template <typename T> class current_calls {
  // pair of operator ID and pointer to session
  std::unordered_map<std::size_t, T> sessions_;

  notify_interface *ni_ptr;
  mutable std::mutex Mut;
  std::size_t N_;

public:
  current_calls();
  current_calls(std::size_t N);
  std::size_t join(T se);
  T operator[](std::size_t);
  void leave(std::size_t opid);
  std::size_t size();
};

template <typename T>
current_calls<T>::current_calls() : ni_ptr(&ni), N_(NCalls) {}

template <typename T>
current_calls<T>::current_calls(std::size_t N) : ni_ptr(&ni), N_(N) {}

template <typename T> std::size_t current_calls<T>::join(T se) {
  std::lock_guard<std::mutex> Lk{Mut};
  for (std::size_t i = 0; i < N_; ++i) {
    if (sessions_.find(i) == sessions_.end()) {
      sessions_.insert({i, se});
      if (size() >= N_)
        ni_ptr->done = false;
      return i;
    }
  }
  return -1;
}

template <typename T> T current_calls<T>::operator[](std::size_t opid) {
  return sessions_[opid];
}

template <typename T> void current_calls<T>::leave(std::size_t opid) {
  std::lock_guard<std::mutex> Lk{ni_ptr->Mut};
  sessions_.erase(opid);
  ni_ptr->done = true;
  ni_ptr->Cond.notify_one();
}

template <typename T> std::size_t current_calls<T>::size() {
  return sessions_.size();
}

#endif
