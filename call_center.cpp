#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <memory>
#include <vector>
#include <thread>
#include <string>

#include "call_center.hpp"
#include "listener.hpp"
#include "ts_queue.hpp"


namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;


class session;

call_center::call_center(const std::string address,
            unsigned short const port,
            int const threads)
    : address_(net::ip::make_address(address)),
      port_(port),
      threads_(threads),
      ioc_(threads)
{
}

int call_center::start()
{
    boost::asio::io_service::work work(ioc_);
    std::make_shared<listener>(
        ioc_,
        tcp::endpoint{address_, port_},
        std::make_shared<current_calls>(),
        std::make_shared< ts_queue< std::shared_ptr<session> > >() )
        ->run();

    std::vector<std::thread> v;
    v.reserve(threads_ - 1);

    for (auto i = threads_ - 1; i > 0; --i)
        v.emplace_back(
            [this]
            {
                ioc_.run();
            });

    ioc_.run();

    return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------

