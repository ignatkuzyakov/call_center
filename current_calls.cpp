#include <memory>
#include <unordered_set>
#include <mutex>

#include "current_calls.hpp"
#include "notify_interface.hpp"

using s_ptr = std::shared_ptr<session>;

current_calls::current_calls() : ni_ptr(&ni) {}

session &current_calls::join(s_ptr se)
{
    std::lock_guard<std::mutex> Lk{Mut};
    return **(sessions_.insert(se).first);
}
void current_calls::leave(s_ptr se)
{
    std::lock_guard<std::mutex> Lk{ni_ptr->Mut};
    sessions_.erase(se);
    ni_ptr->Cond.notify_one();
}

std::size_t current_calls::size()
{
    return sessions_.size();
}
