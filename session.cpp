#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <random>

#include "session.hpp"
#include "current_calls.hpp"
#include "configure.hpp"

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

session::session(
    tcp::socket &&socket,
    std::shared_ptr<current_calls> state)
    : stream_(std::move(socket)),
      state_(state)
{
}

session::~session()
{
}

// Start the asynchronous operation
void session::run()
{
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.

    std::weak_ptr<session> weak(shared_from_this());
    net::dispatch(stream_.get_executor(),
                  [weak]()
                  {
                      std::random_device rd;
                      std::default_random_engine reng(rd());
                      std::uniform_int_distribution<std::size_t> dist(Rmin, Rmax);

                      std::shared_ptr<session> strong(weak);
                      if (strong)
                      {
                        strong->stream_.expires_after(std::chrono::seconds((std::size_t)dist(reng)));
                        
                        strong->do_read();
                      }
                  });
}

void session::do_read()
{
    // Make the request empty before reading,
    // otherwise the operation behavior is undefine d.
    req_ = {};

    // Set the timeout.

    // Read a request
    std::weak_ptr<session> weak(shared_from_this());
    http::async_read(stream_, buffer_, req_, [weak](beast::error_code ec, std::size_t bytes_transferred)
                     {
                             std::shared_ptr<session> strong(weak);
                             if (strong)
                             {
                                 strong->on_read(ec, bytes_transferred);
                             } });
}

void session::on_read(
    beast::error_code ec,
    std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    // This means they closed the connection
    if (ec == http::error::end_of_stream)
        return do_close();

    if (ec)
    {
        state_->leave(shared_from_this());
        return; // fail(ec, "read");
    }

    beast::ostream(res_.body())
        << "CallID: " << make_call_id(req_.target());
    res_.content_length(res_.body().size());
    // Send the response
    send_response();
}

std::string session::make_call_id(boost::beast::string_view number)
{
    return std::to_string(std::atoi(number.data()) >> 2);
}
void session::send_response()
{
    // Write the response
    std::weak_ptr<session> weak(shared_from_this());
    http::async_write(
        stream_,
        res_,
        [weak](beast::error_code ec, std::size_t bytes_transferred)
        {
            std::shared_ptr<session> strong(weak);
            if (strong)
            {
                strong->on_write(ec, bytes_transferred);
            }
        });
}

void session::on_write(
    beast::error_code ec,
    std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec)
        return; // fail(ec, "write");

    // Read another request
    do_read();
}

void session::do_close()
{
    // Send a TCP shutdown
    beast::error_code ec;
    std::weak_ptr<session> weak(shared_from_this());
    state_->leave(weak.lock());
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
    // At this point the connection is closed gracefully
}
