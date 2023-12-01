#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <random>
#include <thread>
#include <chrono>
#include <sstream>

#include "session.hpp"
#include "current_calls.hpp"
#include "configure.hpp"
#include "ts_queue.hpp"
#include "logger.hpp"

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

session::session(
    tcp::socket &&socket,
    std::shared_ptr<current_calls> state,
    std::string DTincoming,
    std::shared_ptr<ts_queue<std::shared_ptr<session>>> qptr)
    : stream_(std::move(socket)),
      state_(state),
      DTIncoming(DTincoming),
      timer(stream_.get_executor()),
      q_ptr(qptr)

{
    CallID = "1234567890";
}

session::~session()
{
    end = boost::posix_time::second_clock::local_time();
    boost::posix_time::time_duration diff = end - start;

    BOOST_LOG_SEV(my_logger::get(), info) << "session:: Destructor";
    BOOST_LOG_SEV(my_logger::get(), info) << "session:: Write CDR";

    if (Status == "overload" || Status == "timeout")
    {
         BOOST_LOG_SEV(my_logger::get(), CDR) <<  DTIncoming + ';' + CallID
                                     + ';' + Number + ';' + to_simple_string(end) 
                                     + ';' + Status + ";;;";
        return;
    }
        BOOST_LOG_SEV(my_logger::get(), CDR) <<  DTIncoming + ';' + CallID
                                     + ';' + Number + ';' + to_simple_string(end) 
                                     + ';' + Status + ';' + to_simple_string(start)
                                     + ';' + std::to_string(OperatorID)
                                     + ';' + std::to_string(diff.total_seconds()) + "s;";
}

void session::set_Status(std::string status)
{
    BOOST_LOG_SEV(my_logger::get(), debug) << "session:: Set Status";

    Status = status;
}

// Start the asynchronous operation
void session::run()
{
    BOOST_LOG_SEV(my_logger::get(), debug) << "session:: Run";
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.

    net::dispatch(stream_.get_executor(),
                  [self = shared_from_this()]()
                  {
                      // Time for operator answer
                      BOOST_LOG_SEV(my_logger::get(), debug) << "session:: Dispatch handler";

                      auto sz = self->q_ptr->size();
                      if (sz >= NQueue)
                      {
                          self->send("queue is full");
                          self->set_Status("overload");

                          BOOST_LOG_SEV(my_logger::get(), warn) << "session:: Queue is full";

                          return;
                      }

                      self->do_read();
                  });
}

void session::get_DT_start_answer(){

                      start = boost::posix_time::second_clock::local_time();
}

void session::check_deadline()
{

    std::weak_ptr<session> weak(shared_from_this());

    timer.async_wait([weak](boost::system::error_code ec)
                     {
        std::shared_ptr<session> strong(weak);
                           
                           
                                          BOOST_LOG_SEV(my_logger::get(), debug) << "session:: Async Wait handler";
        if(strong){
        if (ec) {
                                          BOOST_LOG_SEV(my_logger::get(), error) <<"session:: " << ec.message();

            
            return;}
         
            strong->stream_.cancel();
            if(strong->OperatorID == -1){
                      BOOST_LOG_SEV(my_logger::get(), warn) << "session:: Timeout";

            strong->set_Status("timeout");
            strong->q_ptr->erase(strong->get_CallID());
            }
            else {
                    BOOST_LOG_SEV(my_logger::get(), info) << "session:: Status: OK";
                strong->set_Status("OK");
            }
        } });
}

void session::do_read()
{
    // Make the request empty before reading,
    // otherwise the operation behavior is undefine d.

    req_ = {};
    BOOST_LOG_SEV(my_logger::get(), info) << "session:: Read";

    // Read a request

    http::async_read(stream_, buffer_, req_,
                     [self = shared_from_this()](beast::error_code ec, std::size_t bytes_transferred)
                     {
                         BOOST_LOG_SEV(my_logger::get(), debug) << "session:: Async Read handler";

                         self->on_read(ec, bytes_transferred);
                     });
}

void session::on_read(
    beast::error_code ec,
    std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    // This means they closed the connection
    if (ec)
    {
        BOOST_LOG_SEV(my_logger::get(), error) << "session:: Read: " << ec.message();
        if(ec == http::error::end_of_stream)
        BOOST_LOG_SEV(my_logger::get(), error) << "session:: Abonent closed the connection" << ec.message();
        timer.cancel();
        do_close();
        return;
    }

    Number = req_.target().data();
    std::cout<< Number;
    if (q_ptr->found(Number))
    {
        BOOST_LOG_SEV(my_logger::get(), warn) << "session:: Abonent already in queue";

        send("already in queue");
        return;
    }

    std::string response("You are in queue\nCallID: ");
    response += CallID;
    // Send the response
    send_response(response);
}

std::string session::get_Number()
{
    return Number;
}

void session::start_timer()
{
    std::random_device rd;
    std::default_random_engine reng(rd());
    std::uniform_int_distribution<std::size_t> dist(Rmin, Rmax);

    BOOST_LOG_SEV(my_logger::get(), debug) << "session:: Start Timer";

    timer.expires_from_now(
        boost::posix_time::seconds(
            (std::size_t)dist(reng)));
}

void session::cancel_timer()
{
    BOOST_LOG_SEV(my_logger::get(), debug) << "session:: Cancel Timer";

    timer.cancel();
}

std::string session::get_CallID()
{
    return CallID;
}

void session::send_response(std::string response)
{
    // Prepare the response
    res_.body() = response;
    res_.content_length(response.size());

    BOOST_LOG_SEV(my_logger::get(), info) << "session:: Send Response";

    // Write the response
    http::async_write(
        stream_,
        res_,
        [self = shared_from_this()](beast::error_code ec, std::size_t bytes_transferred)
        {
            BOOST_LOG_SEV(my_logger::get(), debug) << "session:: Send Response Handler";

            self->on_write(ec, bytes_transferred);
        });
}

// Sync send
void session::send(std::string response)
{
    BOOST_LOG_SEV(my_logger::get(), info) << "session:: Send Sync Response";

    res_.body() = response;
    res_.content_length(res_.body().size());
    http::write(
        stream_,
        res_);
}

void session::on_write(
    beast::error_code ec,
    std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec)
    {
        return; // fail(ec, "write");
        BOOST_LOG_SEV(my_logger::get(), error) << "session:: Write" << ec.message();
    }

    BOOST_LOG_SEV(my_logger::get(), debug) << "session:: Push to queue";

    std::weak_ptr<session> weak = shared_from_this();
    q_ptr->push(weak);

    do_read();
}

void session::do_close()
{
    // Send a TCP shutdown
    beast::error_code ec;

    BOOST_LOG_SEV(my_logger::get(), info) << "session:: Do close";
    stream_.cancel();

    if (OperatorID == -1){
 BOOST_LOG_SEV(my_logger::get(), debug) << "session:: Erasing from queue"
                                               << "CallID" << CallID;
        q_ptr->erase(CallID);

    }
    else
    {
        BOOST_LOG_SEV(my_logger::get(), debug) << "session:: "
                                               << "OperatorID: " << OperatorID << " Leave call";
        state_->leave(OperatorID);
    }

    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
    BOOST_LOG_SEV(my_logger::get(), debug) << "session:: Shutdown socket: "
                                           << "OperatorID: " << OperatorID << ' ' << ec.message();

    // At this point the connection is closed gracefully
}

void session::set_OperatorID(std::size_t opid)
{
    OperatorID = opid;
}
