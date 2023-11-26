#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>

#include <iostream>
#include <memory>
#include <thread>
#include <string>
#include <chrono>

#include "current_calls.hpp"
#include "configure.hpp"
#include "session.hpp"
#include "ts_queue.hpp"
#include "listener.hpp"

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

listener::listener(
    net::io_context &ioc,
    tcp::endpoint endpoint, std::shared_ptr<current_calls> state)
    : ioc_(ioc), acceptor_(net::make_strand(ioc)), state_(state)
{
    beast::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if (ec)
    {
        //fail(ec, "open");
        return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec)
    {
        //fail(ec, "set_option");
        return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if (ec)
    {
        //fail(ec, "bind");
        return;
    }

    // Start listening for connections
    acceptor_.listen(
        net::socket_base::max_listen_connections, ec);
    if (ec)
    {
        //fail(ec, "listen");
        return;
    }
}

// Start accepting incoming connections
void listener::run()
{
    std::thread t([this]()
                  {
            for (;;) {
            q.wait_and_pop(state_);
            } });
    t.detach();
    do_accept();
}

void listener::do_accept()
{
    // The new connection gets its own strand
    acceptor_.async_accept(
        net::make_strand(ioc_),
        beast::bind_front_handler(
            &listener::on_accept,
            shared_from_this()));
}

void listener::on_accept(beast::error_code ec, tcp::socket socket)
{
    if (ec)
    {
        //fail(ec, "accept");
        return; // To avoid infinite loop
    }
    else
    {
        // Create the session and run it
        auto sz = q.size();
        if (sz >= NQueue)
        {
            //fail("d");
            do_accept();
        }

        

        q.push(std::make_shared<session>(
            std::move(socket), state_));

    }

    // Accept another connection
    do_accept();
}
