#ifndef TS_QUEUE
#define TS_QUEUE

#include <queue>
#include <memory>
#include <mutex>
#include <thread>

#include "configure.hpp"
#include "current_calls.hpp"
#include "notify_interface.hpp"

template <typename T>
class ts_queue
{
    std::queue<T> Q;
    notify_interface* ni_ptr;

public:
    ts_queue();
    void push(T Data);
    void wait_and_pop(std::shared_ptr<current_calls> state_);
    std::size_t size();

};

template<typename T>
ts_queue<T>::ts_queue() : ni_ptr(&ni){}

template<typename T>
void ts_queue<T>::push(T Data)
{
    std::unique_lock<std::mutex> Lk{ni_ptr->Mut};
    Q.push(std::move(Data));
    ni_ptr->Cond.notify_one();
}

template<typename T>
void ts_queue<T>::wait_and_pop(std::shared_ptr<current_calls> state_)
{
    std::unique_lock<std::mutex> Lk{ni_ptr->Mut};

    ni_ptr->Cond.wait(Lk, [&state_, this]
                      { return (state_->size() < NCalls) && (!Q.empty()); });

    state_->join(std::move(Q.front())).run();
    Q.pop();
}

template<typename T>
std::size_t ts_queue<T>::size()
{
    std::unique_lock<std::mutex> Lk{ni_ptr->Mut};
    return Q.size();
}

#endif