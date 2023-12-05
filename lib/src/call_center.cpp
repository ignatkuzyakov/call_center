#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "../include/call_center.hpp"
#include "../include/listener.hpp"
#include "../include/ts_queue.hpp"

namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

class session;

using current_calls_d = current_calls<std::shared_ptr<session>>;

call_center::call_center(const std::string address, unsigned short const port,
                         int const threads)
    : address_(net::ip::make_address(address)), port_(port), threads_(threads),
      ioc_(threads) {}

int call_center::start() {
  boost::asio::io_service::work work(ioc_);
  std::make_shared<listener>(
      ioc_, tcp::endpoint{address_, port_}, std::make_shared<current_calls_d>(),
      std::make_shared<ts_queue<std::shared_ptr<session>>>())
      ->run();

  std::vector<std::thread> v;
  v.reserve(threads_ - 1);

  for (auto i = threads_ - 1; i > 0; --i)
    v.emplace_back([this] { ioc_.run(); });

  ioc_.run();

  return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------
