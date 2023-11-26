#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <memory>
#include <vector>
#include <thread>
#include <string>

#include "call_center.hpp"
#include "listener.hpp"

namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

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
    std::make_shared<listener>(
        ioc_,
        tcp::endpoint{address_, port_},
        std::make_shared<current_calls>())
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

