
#include <iostream>
#include <boost/locale/generator.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/support/date_time.hpp>

#include "logger.hpp"

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;
    
BOOST_LOG_GLOBAL_LOGGER_INIT(my_logger, src::severity_logger_mt<severity_level>) {
    src::severity_logger_mt<severity_level> lg;
    logging::add_common_attributes();
    logging::add_file_log
    (
        keywords::file_name = "log.txt",
        keywords::auto_flush = true,
        keywords::format = (expr::stream
            << "["<<expr::format_date_time(timestamp, "%Y-%m-%d %H:%M:%S.%f")
            << "] ["
            <<expr::attr<attrs::current_thread_id::value_type>("ThreadID") <<"]"
            << " [" << severity
            << "] " << expr::message
            )
    );
    return lg;
}

template< typename CharT, typename TraitsT >
inline std::basic_ostream< CharT, TraitsT >& operator<< (
    std::basic_ostream< CharT, TraitsT >& strm, severity_level lvl)
{
    static const char* const str[] =
    {
        "debug",
        "info",
        "warn",
        "critical",
        "error"
    };
    if (static_cast< std::size_t >(lvl) < (sizeof(str) / sizeof(*str)))
        strm << str[lvl];
    else
        strm << static_cast< int >(lvl);
    return strm;
}



