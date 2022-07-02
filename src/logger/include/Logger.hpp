#ifndef LOGGER_HPP
#define LOGGER_HPP

#define BOOST_LOG_DYN_LINK 1

#include <stdexcept>
#include <string>
#include <iostream>
#include <fstream>
#include <functional>
#include <boost/ref.hpp>
#include <boost/bind/bind.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/record_ordering.hpp>
#include <boost/log/support/date_time.hpp>

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

namespace logging  = boost::log;
namespace attrs    = logging::attributes;
namespace src      = logging::sources;
namespace sinks    = logging::sinks;
namespace expr     = logging::expressions;
namespace keywords = logging::keywords;
namespace trivial  = logging::trivial;

#define INFO  trivial::severity_level::info
#define DEBUG trivial::severity_level::debug
#define TRACE trivial::severity_level::trace
#define WARN  trivial::severity_level::warning
#define ERR   trivial::severity_level::error
#define FATAL trivial::severity_level::fatal

using boost::shared_ptr, std::string;

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(
    logger, src::severity_logger<trivial::severity_level>)

#define lg logger::get()

#define LOG_INF BOOST_LOG_SEV(lg, INFO)
#define LOG_DBG BOOST_LOG_SEV(lg, DEBUG)
#define LOG_ERR BOOST_LOG_SEV(lg, ERR)

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

namespace Logging
{
    typedef sinks::text_ostream_backend backend_t;
    typedef sinks::asynchronous_sink<
        backend_t,
        sinks::unbounded_ordering_queue<
            logging::attribute_value_ordering<uint16_t, std::less<uint16_t>>>>
                              sink_t;
    extern shared_ptr<sink_t> sink;
    void                      initLogger(string fileName, bool auto_flush);
    void                      flushLogs();
} // namespace Logging
#endif

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/