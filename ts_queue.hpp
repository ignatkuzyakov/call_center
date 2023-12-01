#ifndef TS_QUEUE_HPP
#define TS_QUEUE_HPP

#include <queue>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>

#include "configure.hpp"
#include "current_calls.hpp"
#include "notify_interface.hpp"
#include "session.hpp"

template <typename T>
class ts_queue
{
    std::deque<T> Q;
    notify_interface *ni_ptr;
 
public:
    ts_queue();
    void push(std::weak_ptr<session>);
    void wait_and_pop(std::shared_ptr<current_calls>);
    std::size_t size();

    void erase(std::string);

    bool found(std::string);
};

template <typename T>
ts_queue<T>::ts_queue() : ni_ptr(&ni) {}

template <typename T>
void ts_queue<T>::push(std::weak_ptr<session> weak)
{
    std::unique_lock<std::mutex> Lk{ni_ptr->Mut};
    Q.push_back(weak.lock());
    Q.front()->start_timer();
    Q.front()->check_deadline();
    ni_ptr->Cond.notify_one();
}

template <typename T>
void ts_queue<T>::erase(std::string CallID)
{
    std::lock_guard<std::mutex> Lk{ni_ptr->Mut};
    typename std::deque<T>::iterator sta, fin;

    for (sta = Q.begin(), fin = Q.end(); sta != fin; ++sta)
    {
        if ((*sta)->get_CallID() == CallID)
        {
            Q.erase(sta);
            return;
        }
    }
}

template <typename T>
void ts_queue<T>::wait_and_pop(std::shared_ptr<current_calls> state_)
{
    std::unique_lock<std::mutex> Lk{ni_ptr->Mut};
    ni_ptr->Cond.wait(Lk, [&state_, this]
                      { return (state_->size() < NCalls) && (!Q.empty()); });

    auto opid = state_->join(std::move(Q.front()));
    auto sptr = (*state_)[opid];

    sptr->set_OperatorID(opid);
    sptr->send("We are talking");
    sptr->get_DT_start_answer();
    sptr->cancel_timer();
    sptr->start_timer();
    sptr->check_deadline();

    Q.pop_front();
}

template <typename T>
bool ts_queue<T>::found(std::string num)
{
    std::unique_lock<std::mutex> Lk{ni_ptr->Mut};
    for (auto x : Q)
    {
        if (x->get_Number() == num)
            return true;
    }
    return false;
}

template <typename T>
std::size_t ts_queue<T>::size()
{
    std::unique_lock<std::mutex> Lk{ni_ptr->Mut};
    return Q.size();
}

#endif
