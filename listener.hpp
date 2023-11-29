#ifndef LISTENER
#define LISTENER

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <boost/log/sources/logger.hpp>


#include <memory>
#include <unordered_map>

#include "current_calls.hpp"
#include "session.hpp"
#include "ts_queue.hpp"
#include "logger.hpp"


namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

using logger = src::severity_logger<severity_level>;
// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
    net::io_context &ioc_;
    tcp::acceptor acceptor_;

    std::shared_ptr<ts_queue<std::shared_ptr<session>>> q_ptr;

    std::shared_ptr<current_calls> state_;

public:
    listener(
        net::io_context &ioc,
        tcp::endpoint endpoint, 
        std::shared_ptr<current_calls> state,
        std::shared_ptr<ts_queue<std::shared_ptr<session>>>
        );
    
    // Start accepting incoming connections
    void run();

private:
    void do_accept();
    void on_accept(beast::error_code ec, tcp::socket socket);
};

#endif
