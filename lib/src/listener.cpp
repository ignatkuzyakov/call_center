#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <memory>
#include <thread>

#include "../include/current_calls.hpp"
#include "../include/listener.hpp"
#include "../include/logger.hpp"
#include "../include/session.hpp"
#include "../include/ts_queue.hpp"

using current_calls_d = current_calls<std::shared_ptr<session>>;

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

listener::listener(net::io_context &ioc, tcp::endpoint endpoint,
                   std::shared_ptr<current_calls_d> state,
                   std::shared_ptr<ts_queue<std::shared_ptr<session>>> qptr)
    : ioc_(ioc), acceptor_(net::make_strand(ioc)), state_(state), q_ptr(qptr) {
  BOOST_LOG_SEV(lg, debug) << "[listener] Constructing...";
  beast::error_code ec;
  BOOST_LOG_SEV(lg, info) << "[listener] Opening the acceptor...";
  acceptor_.open(endpoint.protocol(), ec);
  if (ec) {
    BOOST_LOG_SEV(lg, critical)
        << "[listener] Open the acceptor " << ec.message();
    return;
  } else {
    BOOST_LOG_SEV(lg, info) << "[listener] Open the acceptor " << ec.message();
  }
  BOOST_LOG_SEV(lg, info) << "[listener] Setting options...";
  acceptor_.set_option(net::socket_base::reuse_address(true), ec);
  if (ec) {
    BOOST_LOG_SEV(lg, critical) << "[listener] Setting options" << ec.message();
    return;
  } else {
    BOOST_LOG_SEV(lg, info) << "[listener] Setting options " << ec.message();
  }
  BOOST_LOG_SEV(lg, info) << "[listener] Binding server address...";
  acceptor_.bind(endpoint, ec);
  if (ec) {
    BOOST_LOG_SEV(lg, critical)
        << "[listener] Binding server address" << ec.message();
    return;
  } else {
    BOOST_LOG_SEV(lg, info)
        << "[listener] Binding server address " << ec.message();
  }

  BOOST_LOG_SEV(lg, info) << "[listener] Starting listening for connections...";
  acceptor_.listen(net::socket_base::max_listen_connections, ec);
  if (ec) {
    BOOST_LOG_SEV(lg, critical)
        << "[listener] listening for connections..." << ec.message();
    return;
  } else {
    BOOST_LOG_SEV(lg, info)
        << "[listener] listening for connections..." << ec.message();
  }

  BOOST_LOG_SEV(lg, info) << "[listener] Construct Success";
}

// Start accepting incoming connections
void listener::run() {
  BOOST_LOG_SEV(lg, info) << "[listener] Running...";
  std::thread t([this]() {
    BOOST_LOG_SEV(lg, info) << "[listener] Make thread for queue handler";

    for (;;) {
      BOOST_LOG_SEV(lg, debug) << "[listener] Call wait and pop";
      std::shared_ptr<session> sptr;
      q_ptr->wait_and_pop(sptr);
      auto opid = state_->join(sptr);
      sptr->send("We are talking");
      sptr->get_DT_start_answer();
      sptr->cancel_timer();
      sptr->start_timer();
      sptr->check_deadline();
      sptr->set_OperatorID(opid);
    }
  });
  t.detach();
  do_accept();
}

void listener::do_accept() {
  BOOST_LOG_SEV(lg, info) << "[listener] Accept new connection";
  // The new connection gets its own strand
  acceptor_.async_accept(
      net::make_strand(ioc_),
      beast::bind_front_handler(&listener::on_accept, shared_from_this()));
}

void listener::on_accept(beast::error_code ec, tcp::socket socket) {
  if (ec) {
    BOOST_LOG_SEV(lg, critical)
        << "[listener] Can't Accept new connection" << ec.message();
    return; // To avoid infinite loop
  } else {
    BOOST_LOG_SEV(lg, info)
        << "[listener] Accepting new connection " << ec.message();
    auto now = boost::posix_time::second_clock::local_time();

    // Create the session and run it
    BOOST_LOG_SEV(lg, info) << "[listener] Making New session...";
    std::make_shared<session>(std::move(socket), state_, to_simple_string(now),
                              q_ptr)
        ->run();
  }

  // Accept another connections
  do_accept();
}
