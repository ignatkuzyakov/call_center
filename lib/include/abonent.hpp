#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <memory>

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

// Report a failure
void fail(beast::error_code ec, char const *what);

// Performs an HTTP GET and prints the response
class ASession : public std::enable_shared_from_this<ASession> {
  tcp::resolver resolver_;
  beast::tcp_stream stream_;
  beast::flat_buffer buffer_; // (Must persist between reads)
  http::request<http::empty_body> req_;
  http::response<http::string_body> res_;

public:
  // Objects are constructed with a strand to
  // ensure that handlers do not execute concurrently.
  explicit ASession(net::io_context &ioc);

  // Start the asynchronous operation
  void run(char const *host, char const *port, char const *target, int version);
  void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
  void on_connect(beast::error_code ec,
                  tcp::resolver::results_type::endpoint_type);

  void on_write(beast::error_code ec, std::size_t bytes_transferred);

  void on_read(beast::error_code ec, std::size_t bytes_transferred);
};

class abonent {
  std::string host;
  std::string port;
  std::string number;
  int version;

  net::io_context ioc;

public:
  abonent(std::string, std::string, std::string, int);
  int call();
};
