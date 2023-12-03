#ifndef SESSION_HPP
#define SESSION_HPP

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

#include <memory>
#include <string>

#include "current_calls.hpp"
#include "ts_queue.hpp"

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

class current_calls;
template <typename T>
class ts_queue;

class session : public std::enable_shared_from_this<session> {
  beast::tcp_stream stream_;
  beast::flat_buffer buffer_;

  http::request<http::dynamic_body> req_;
  http::response<http::string_body> res_;

  std::shared_ptr<current_calls> state_;
  std::shared_ptr<ts_queue<std::shared_ptr<session>>> q_ptr;

  int OperatorID = -1;

  std::string CallID;
  std::string DTIncoming;
  std::string Status;
  std::string Number;

  boost::asio::deadline_timer timer;
  boost::posix_time::ptime start, end;

public:
  // Take ownership of the stream
  session(tcp::socket &&, std::shared_ptr<current_calls>, std::string,
          std::shared_ptr<ts_queue<std::shared_ptr<session>>>);

  ~session();

  void run();
  void check_deadline();
  void do_read();

  void start_timer();
  void cancel_timer();
  void get_DT_start_answer();
  void on_read(beast::error_code ec, std::size_t bytes_transferred);

  void set_Status(std::string);
  void set_OperatorID(std::size_t);

  std::string get_Number();
  std::string get_CallID();

  void send_response(std::string);
  void send(std::string);
  void on_write(beast::error_code ec, std::size_t bytes_transferred);

  void do_close();
};

#endif
