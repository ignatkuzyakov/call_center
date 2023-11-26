#ifndef CURRENT_CALLS
#define CURRENT_CALLS

#include <memory>
#include <unordered_set>
#include <thread>
#include <string>

#include "notify_interface.hpp"

class session;

using s_ptr = std::shared_ptr<session>;

class current_calls
{
    std::unordered_set<s_ptr> sessions_;
    notify_interface* ni_ptr;
    mutable std::mutex Mut;
    
public:
    current_calls();
    session& join(s_ptr se);
    void leave(s_ptr se);
    std::size_t size();
};

#endif