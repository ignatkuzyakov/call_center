#ifndef NOTIFY_INTERFACE_HPP
#define NOTIFY_INTERFACE_HPP

#include <condition_variable>

extern struct notify_interface {

  mutable std::mutex Mut;
  std::condition_variable Cond;

} ni;

#endif
