#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/lexical_cast.hpp>

#include <cstddef>
#include <memory>
#include <random>
#include <string>

#include "../include/configure.hpp"
#include "../include/current_calls.hpp"
#include "../include/logger.hpp"
#include "../include/session.hpp"
#include "../include/ts_queue.hpp"

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

using current_calls_d = current_calls<std::shared_ptr<session>>;

session::session(tcp::socket &&socket, std::shared_ptr<current_calls_d> state,
                 std::string DTincoming,
                 std::shared_ptr<ts_queue<std::shared_ptr<session>>> qptr)
    : stream_(std::move(socket)), state_(state), DTIncoming(DTincoming),
      timer(stream_.get_executor()), q_ptr(qptr)

{
  std::random_device rd;
  std::default_random_engine reng(rd());
  std::uniform_int_distribution<std::size_t> dist(100000, 999999);
  CallID = std::to_string((std::size_t)dist(reng));
  BOOST_LOG_SEV(lg, info) << "[session] CallID[" + CallID + "] Construct";
}

session::~session() {
  end = boost::posix_time::second_clock::local_time();
  boost::posix_time::time_duration diff = end - start;

  BOOST_LOG_SEV(lg, info) << "[session] Number[" + Number + "] CallID[" +
                                 CallID + "] Destructor";
  if (Status == "already in queue")
    return;
  BOOST_LOG_SEV(lg, info) << "[session] Number[" + Number + "] CallID[" +
                                 CallID + "] Write CDR";

  if (Status == "overload" || Status == "timeout") {
    BOOST_LOG_SEV(lg, CDR) << DTIncoming + ';' + CallID + ';' + Number + ';' +
                                  to_simple_string(end) + ';' + Status + ";;;";
    return;
  }
  BOOST_LOG_SEV(lg, CDR) << DTIncoming + ';' + CallID + ';' + Number + ';' +
                                to_simple_string(end) + ';' + Status + ';' +
                                to_simple_string(start) + ';' +
                                std::to_string(OperatorID) + ';' +
                                std::to_string(diff.total_seconds()) + "s;";
}

void session::set_Status(std::string status) {
  BOOST_LOG_SEV(lg, debug) << "[session] Number[" + Number + "] CallID[" +
                                  CallID + "] Set status " + status;
  Status = status;
}

// Start the asynchronous operation
void session::run() {
  BOOST_LOG_SEV(lg, info) << "[session] CallID[" + CallID + "] Run";
  // We need to be executing within a strand to perform async operations
  // on the I/O objects in this session. Although not strictly necessary
  // for single-threaded contexts, this example code is written to be
  // thread-safe by default.

  net::dispatch(stream_.get_executor(), [self = shared_from_this()]() {
    // Time for operator answer

    BOOST_LOG_SEV(self->lg, info)
        << "[session] CallID[" + self->CallID + "] Dispatch handler";

    self->do_read();
  });
}

void session::get_DT_start_answer() {
  start = boost::posix_time::second_clock::local_time();
}

void session::check_deadline() {

  std::weak_ptr<session> weak(shared_from_this());

  timer.async_wait([weak](boost::system::error_code ec) {
    std::shared_ptr<session> strong(weak);

    BOOST_LOG_SEV(strong->lg, debug) << "[session] Number[" + strong->Number +
                                            "] CallID[" + strong->CallID +
                                            "] Status: Async wait handler";
    if (strong) {
      if (ec) {
        BOOST_LOG_SEV(strong->lg, error)
            << "[session] Number[" + strong->Number + "] CallID[" +
                   strong->CallID + "] ERROR: " + ec.message();
        return;
      }

      strong->stream_.cancel();
      if (strong->OperatorID == -1) {
        BOOST_LOG_SEV(strong->lg, warn)
            << "[session] Number[" + strong->Number + "] CallID[" +
                   strong->CallID + "] Status timeout";

        strong->set_Status("timeout");
        strong->q_ptr->erase(strong);
      } else {
        BOOST_LOG_SEV(strong->lg, info) << "[session] Number[" +
                                               strong->Number + "] CallID[" +
                                               strong->CallID + "] Status OK";
        strong->set_Status("OK");
      }
    }
  });
}

void session::do_read() {
  // Make the request empty before reading,
  // otherwise the operation behavior is undefine d.
  req_ = {};
  BOOST_LOG_SEV(lg, info) << "[session] Number[" + Number + "] CallID[" +
                                 CallID + "] Read";
  // Read a request
  http::async_read(stream_, buffer_, req_,
                   [self = shared_from_this()](beast::error_code ec,
                                               std::size_t bytes_transferred) {
                     BOOST_LOG_SEV(self->lg, debug)
                         << "[session] Number[" + self->Number + "] CallID[" +
                                self->CallID + "] Async read handler";

                     self->on_read(ec, bytes_transferred);
                   });
}

void session::on_read(beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  // This means they closed the connection
  if (ec) {
    BOOST_LOG_SEV(lg, critical) << "[session] Number[" + Number + "] CallID[" +
                                       CallID + "] Read " + ec.message();
    if (ec == http::error::end_of_stream)
      BOOST_LOG_SEV(lg, critical) << "[session] Number[" + Number +
                                         "] CallID[" + CallID +
                                         "] Abonent close connection";
    timer.cancel();
    do_close();
    return;
  }

  Number = req_.target().data();

  if (q_ptr->found_Number(shared_from_this())) {
    BOOST_LOG_SEV(lg, warn) << "[session] Number[" + Number + "] CallID[" +
                                   CallID + "] Already in queue";
    send("already in queue");
    set_Status("already in queue");
    return;
  }

  auto sz = q_ptr->size();
  if (sz >= NQueue) {
    send("queue is full");
    set_Status("overload");
    BOOST_LOG_SEV(lg, warn) << "[session] Number[" + Number + "] CallID[" +
                                   CallID + "] Queue is full";
    return;
  }
  std::string response("in queue\nCallID: ");
  response += CallID;
  // Send the response
  send_response(response);
}

std::string session::get_Number() { return Number; }

void session::start_timer() {
  std::random_device rd;
  std::default_random_engine reng(rd());
  std::uniform_int_distribution<std::size_t> dist(Rmin, Rmax);

  BOOST_LOG_SEV(lg, info) << "[session] Number[" + Number + "] CallID[" +
                                 CallID + "] Start timer";
  timer.expires_from_now(boost::posix_time::seconds((std::size_t)dist(reng)));
}

void session::cancel_timer() {
  BOOST_LOG_SEV(lg, info) << "[session] Number[" + Number + "] CallID[" +
                                 CallID + "] Cancel timer";
  timer.cancel();
}

std::string session::get_CallID() { return CallID; }

void session::send_response(std::string response) {
  // Prepare the response
  res_.body() = response;
  res_.content_length(response.size());

  BOOST_LOG_SEV(lg, info) << "[session] Number[" + Number + "] CallID[" +
                                 CallID + "] Send response";

  // Write the response
  http::async_write(stream_, res_,
                    [self = shared_from_this()](beast::error_code ec,
                                                std::size_t bytes_transferred) {
                      BOOST_LOG_SEV(self->lg, debug)
                          << "[session] Number[" + self->Number + "] CallID[" +
                                 self->CallID + "] Write handler response";

                      self->on_write(ec, bytes_transferred);
                    });
}

// Sync send TODO: REMOVE!
void session::send(std::string response) {
  BOOST_LOG_SEV(lg, info) << "[session] Number[" + Number + "] CallID[" +
                                 CallID + "] Send sync response";

  res_.body() = response;
  res_.content_length(res_.body().size());
  http::write(stream_, res_);
}

void session::on_write(beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec) {
    return; // fail(ec, "write");
    BOOST_LOG_SEV(lg, error) << "[session] Number[" + Number + "] CallID[" +
                                    CallID + "] Write " + ec.message();
  }

  BOOST_LOG_SEV(lg, info) << "[session] Number[" + Number + "] CallID[" +
                                 CallID + "] Push to queue";

  q_ptr->push(shared_from_this());
  start_timer();
  check_deadline();
  do_read();
}

void session::do_close() {
  // Send a TCP shutdown
  beast::error_code ec;

  BOOST_LOG_SEV(lg, info) << "[session] Number[" + Number + "] CallID[" +
                                 CallID + "] Do close";
  stream_.cancel();

  if (OperatorID == -1) {
    BOOST_LOG_SEV(lg, debug) << "[session] Number[" + Number + "] CallID[" +
                                    CallID + "] Erasing from queue";
    q_ptr->erase(shared_from_this());

  } else {
    BOOST_LOG_SEV(lg, debug) << "[session] Number[" + Number + "] CallID[" +
                                    CallID + "] Leave Call With OperatorID "
                             << OperatorID;
    state_->leave(OperatorID);
  }
  stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
  BOOST_LOG_SEV(lg, debug) << "[session] Number[" + Number + "] CallID[" +
                                  CallID + "] Leave Call With OperatorID "
                           << OperatorID << " Socket shutdown: " + ec.message();
  // At this point the connection is closed gracefully
}

void session::set_OperatorID(std::size_t opid) {
  BOOST_LOG_SEV(lg, debug) << "[session] Number[" + Number + "] CallID[" +
                                  CallID + "] Join Call With OperatorID "
                           << opid;
  OperatorID = opid;
}
