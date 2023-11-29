#ifndef CURRENT_CALLS_HPP
#define CURRENT_CALLS_HPP

#include <memory>
#include <unordered_map>
#include <thread>
#include <string>

#include "notify_interface.hpp"

class session;

using s_ptr = std::shared_ptr<session>;

class current_calls
{
    //pair of operator ID and pointer to session
    std::unordered_map<std::size_t, s_ptr> sessions_;

    notify_interface* ni_ptr;
    mutable std::mutex Mut;
    
public:
    current_calls();
    std::size_t join(s_ptr se);
    s_ptr operator[](std::size_t);
    void leave(std::size_t opid);
    std::size_t size();
};

#endif
