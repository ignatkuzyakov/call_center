#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "../include/abonent.hpp"

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

void fail(beast::error_code ec, char const *what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

ASession::ASession(net::io_context &ioc)
    : resolver_(net::make_strand(ioc)), stream_(net::make_strand(ioc)) {}

void ASession::run(char const *host, char const *port, char const *target,
                   int version) {
  req_.version(version);
  req_.method(http::verb::get);
  req_.target(target);
  req_.set(http::field::host, host);
  req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  // Look up the domain name
  resolver_.async_resolve(
      host, port,
      beast::bind_front_handler(&ASession::on_resolve, shared_from_this()));
}

void ASession::on_resolve(beast::error_code ec,
                          tcp::resolver::results_type results) {
  if (ec)
    return fail(ec, "resolve");

  // Make the connection on the IP address we get from a lookup
  stream_.async_connect(
      results,
      beast::bind_front_handler(&ASession::on_connect, shared_from_this()));
}

void ASession::on_connect(beast::error_code ec,
                          tcp::resolver::results_type::endpoint_type) {
  if (ec)
    return fail(ec, "connect");

  // Send the HTTP request to the remote host
  http::async_write(
      stream_, req_,
      beast::bind_front_handler(&ASession::on_write, shared_from_this()));
}

void ASession::on_write(beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec)
    return fail(ec, "write");

  // Receive the HTTP response
  http::async_read(
      stream_, buffer_, res_,
      beast::bind_front_handler(&ASession::on_read, shared_from_this()));
}

void ASession::on_read(beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec)
    return;

  // Write the message to standard out
  std::cout << res_.body().c_str() << std::endl;
  res_.body() = {};
  buffer_.clear();
  http::async_read(
      stream_, buffer_, res_,
      beast::bind_front_handler(&ASession::on_read, shared_from_this()));
}

abonent::abonent(std::string h, std::string p, std::string n, int v)
    : host(h), port(p), number(n), version(v) {}

int abonent::call() {
  std::make_shared<ASession>(ioc)->run(host.c_str(), port.c_str(),
                                       number.c_str(), version);
  ioc.run();
  return EXIT_SUCCESS;
}
