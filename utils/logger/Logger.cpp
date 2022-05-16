#include "Logger.hpp"

using namespace Logging;

shared_ptr<sink_t> Logging::sink;

// Initializes the logger
void Logging::initLogger(string fileName, bool auto_flush)
{
    try
    {
        shared_ptr<std::ostream> strm(new std::ofstream(fileName.c_str()));
        if (!strm->good())
        {
            throw std::runtime_error("Failed to open a text log file");
        }
        shared_ptr<backend_t> backend = boost::make_shared<backend_t>();
        backend->auto_flush(auto_flush);
        sink = boost::make_shared<sink_t>(
            backend, keywords::order = logging::make_attr_ordering(
                         "RecordID", std::less<unsigned int>()));

        sink->locked_backend()->add_stream(strm);
        sink->set_formatter(
            expr::format("[%1%] [%2%] <%3%> : %4%") %
            expr::format_date_time<boost::posix_time::ptime>("TimeStamp",
                                                             "%H:%M:%S.%f") %
            expr::attr<attrs::current_thread_id::value_type>("ThreadID") %
            logging::trivial::severity % expr::smessage);

        boost::shared_ptr<logging::core> core = logging::core::get();
        
        core->add_sink(sink);
        core->add_global_attribute("TimeStamp", attrs::local_clock());
        core->add_global_attribute("ThreadID", attrs::current_thread_id());
    }
    catch (const std::exception &e)
    {
        std::cerr << "FAILED TO INIT LOGGER" << e.what() << '\n';
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void Logging::flushLogs()
{
    sink->flush();
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/
