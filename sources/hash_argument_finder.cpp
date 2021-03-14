// Copyright 2020 LabradorWithShades
// 70892548+LabradorWithShades@users.noreply.github.com

#include <PicoSHA2/picosha2.h>

#include <boost/core/null_deleter.hpp>

#include <boost/log/core.hpp>

#include <boost/log/expressions.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>

#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>

#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>

#include <boost/log/support/date_time.hpp>

#include <boost/log/utility/setup/common_attributes.hpp>

#include <boost/phoenix.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

#include <hash_argument_finder.hpp>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

enum severity_level { debug, trace, info };
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", severity_level)

std::ostream& operator<<(std::ostream& strm, severity_level level) {
  static const char* strings[] = {"debug",   "trace", "info",
                                  "warning", "error", "critical"};

  if (static_cast<std::size_t>(level) < sizeof(strings) / sizeof(*strings))
    strm << strings[level];
  else
    strm << static_cast<int>(level);

  return strm;
}

static bool checkHash(std::string hash) {
  size_t last = hash.size() - 1;
  if (last < 3) return false;
  if (hash[last] == '0')
    if (hash[last - 1] == '0')
      if (hash[last - 2] == '0')
        if (hash[last - 3] == '0') return true;
  return false;
}

static void worker(ReportStorage* reportStorage,
                   [[maybe_unused]] uint32_t threadID,
                   const std::shared_ptr<std::atomic_bool>& stopSign) {
  src::severity_logger<severity_level> slg;
  bool shouldStop = stopSign->load();
  while (!shouldStop) {
    std::stringstream stringStream;
    stringStream << std::hex << std::rand();
    std::string data = stringStream.str();
    std::string hash{};
    picosha2::hash256_hex_string(data, hash);
    BOOST_LOG_SEV(slg, trace) << "[Thread:" << threadID << "] data = \"" << data
                              << "\", hash = \"" << hash << "\"";
    if (checkHash(hash)) {
      BOOST_LOG_SEV(slg, info)
          << "[Thread:" << threadID << "] Found suitable hash with data = \""
          << data << "\" and hash = \"" << hash << "\"";
      reportStorage->addEntry(time(nullptr), hash, data);
    }
    shouldStop = stopSign->load();
  }
  BOOST_LOG_SEV(slg, debug) << "[Thread:" << threadID << "] stopped";
}

bool my_filter(logging::value_ref<severity_level, tag::severity> const& level) {
  return level >= info;
}

HashArgumentFinder::HashArgumentFinder() noexcept
    : m_storage(),
      m_saveFileName(),
      m_threads(),
      m_threadsStopSigns(),
      m_threadCount(0),
      m_started(false) {
  logging::add_common_attributes();

  using sink_file_back_t = sinks::text_file_backend;
  using file_sink_t = sinks::synchronous_sink<sink_file_back_t>;

  boost::shared_ptr<sink_file_back_t> file_backend =
      boost::make_shared<sink_file_back_t>(
          keywords::file_name = "hash_arg_finder_%5N.log",
          keywords::rotation_size = 100 * 1024 * 1024);
  boost::shared_ptr<file_sink_t> file_sink(new file_sink_t(file_backend));

  using sink_stream_back_t = sinks::text_ostream_backend;
  using stream_sink_t = sinks::synchronous_sink<sink_stream_back_t>;

  boost::shared_ptr<sink_stream_back_t> stream_backend =
      boost::make_shared<sink_stream_back_t>();
  boost::shared_ptr<std::ostream> stream(&std::cout, boost::null_deleter());
  stream_backend->add_stream(stream);

  boost::shared_ptr<stream_sink_t> stream_sink(
      new stream_sink_t(stream_backend));

  namespace phoenix = boost::phoenix;
  stream_sink->set_filter(phoenix::bind(my_filter, severity.or_none()));

  namespace expr = logging::expressions;
  logging::formatter fmt = expr::stream
                           << "["
                           << expr::format_date_time<boost::posix_time::ptime>(
                                  "TimeStamp", "%Y-%m-%d %H:%M:%S")
                           << "]"
                           << "[" << severity << "\t]" << expr::smessage;

  file_sink->set_formatter(fmt);
  stream_sink->set_formatter(fmt);

  logging::core::get()->add_sink(file_sink);
  logging::core::get()->add_sink(stream_sink);
}

void HashArgumentFinder::startSearch(std::string fileName, size_t count) {
  if (m_started) return;

  m_saveFileName = std::move(fileName);
  m_threads.resize(count);
  m_threadsStopSigns.resize(count);
  m_threadCount = count;

  for (uint32_t i = 0; i < m_threadCount; ++i) {
    m_threadsStopSigns[i] = std::make_shared<std::atomic_bool>(false);
    m_threads[i] = std::make_shared<std::thread>(worker, &m_storage, i,
                                                 m_threadsStopSigns[i]);
  }

  m_started = true;
}

void HashArgumentFinder::stopSearch() {
  if (!m_started) return;

  for (uint32_t i = 0; i < m_threadCount; ++i) {
    m_threadsStopSigns[i]->store(true);
    m_threads[i]->join();
  }
  m_storage.exportToFile(m_saveFileName);

  m_started = false;
}

HashArgumentFinder::~HashArgumentFinder() {
  stopSearch();
}
