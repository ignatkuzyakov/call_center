#ifndef TS_QUEUE_HPP
#define TS_QUEUE_HPP

#include <deque>
#include <mutex>

#include "notify_interface.hpp"
#include "session.hpp"

template <typename T> class ts_queue {
  std::deque<T> Q;
  notify_interface *ni_ptr;

public:
  ts_queue();
  ts_queue(notify_interface &);
  void push(T);
  void wait_and_pop(T &);
  std::size_t size();

  void erase(T);

  bool found_Number(T);
};

template <typename T> ts_queue<T>::ts_queue(notify_interface &n) : ni_ptr(&n) {}

template <typename T> ts_queue<T>::ts_queue() : ni_ptr(&ni) {}

template <typename T> void ts_queue<T>::push(T Data) {
  std::unique_lock<std::mutex> Lk{ni_ptr->Mut};
  Q.push_back(std::move(Data));
  ni_ptr->Cond.notify_one();
}

template <typename T> void ts_queue<T>::erase(T Data) {
  std::lock_guard<std::mutex> Lk{ni_ptr->Mut};
  typename std::deque<T>::iterator sta, fin;

  for (sta = Q.begin(), fin = Q.end(); sta != fin; ++sta) {
    if (*sta == Data) {
      Q.erase(sta);
      return;
    }
  }
}

template <typename T> void ts_queue<T>::wait_and_pop(T &Data) {
  std::unique_lock<std::mutex> Lk{ni_ptr->Mut};
  ni_ptr->Cond.wait(Lk, [this] { return ni_ptr->done && (!Q.empty()); });
  Data = std::move(Q.front());
  Q.pop_front();
}

template <typename T> bool ts_queue<T>::found_Number(T Data) {
  std::unique_lock<std::mutex> Lk{ni_ptr->Mut};
  for (auto x : Q) {
    if (x->get_Number() == Data->get_Number())
      return true;
  }
  return false;
}

template <typename T> std::size_t ts_queue<T>::size() {
  std::unique_lock<std::mutex> Lk{ni_ptr->Mut};
  return Q.size();
}

#endif
