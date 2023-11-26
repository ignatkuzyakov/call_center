#ifndef SESSION
#define SESSION

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <string>
#include <memory>
#include <chrono>

#include "current_calls.hpp"

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

class current_calls;

class session : public std::enable_shared_from_this<session>
{
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    http::request<http::dynamic_body> req_;
    http::response<http::dynamic_body> res_;
    std::shared_ptr<current_calls> state_;
    std::string CallID;
    std::string CDR;

    std::chrono::system_clock::time_point start, end;

public:
    // Take ownership of the stream
    session(
        tcp::socket &&socket,
        std::shared_ptr<current_calls> state);

    ~session();

    void run();

    void do_read();

    void on_read(
        beast::error_code ec,
        std::size_t bytes_transferred);

    void send_response();

    void on_write(
        beast::error_code ec,
        std::size_t bytes_transferred);

    void do_close();
};

#endif