#ifndef CALL_CENTER_HPP
#define CALL_CENTER_HPP

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>

namespace net = boost::asio;

class call_center {
  boost::asio::ip::address address_;
  unsigned short const port_;
  int const threads_;
  net::io_context ioc_;

public:
  call_center(const std::string address, unsigned short const port,
              int const threads);

  int start();
};

#endif
