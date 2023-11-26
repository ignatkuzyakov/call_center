#ifndef NOTIFY_INTERFACE
#define NOTIFY_INTERFACE

#include <thread>
#include <condition_variable>

extern struct notify_interface {

    mutable std::mutex Mut;
    std::condition_variable Cond;

} ni;

#endif