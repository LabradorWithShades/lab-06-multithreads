// Copyright 2020 LabradorWithShades
// 70892548+LabradorWithShades@users.noreply.github.com

#include <PicoSHA2/picosha2.h>

#include <hash_argument_finder.hpp>
#include <utility>

static bool checkHash([[maybe_unused]] std::string hash) {
  return true;
}

static void worker(ReportStorage* reportStorage, uint32_t threadID,
                   std::atomic<bool>* stopSign) {
  while (!stopSign->load()) {
    std::string data = std::to_string(std::rand());
    std::string hash{};
    picosha2::hash256_hex_string(data, hash);
    if (checkHash(hash)) {// doest it satisfy mask?
      reportStorage->addEntry(time(NULL), hash, data);
    }
    // discard it
  }
}

HashArgumentFinder::HashArgumentFinder(std::string fileName, uint32_t count)
    : m_storage(),
      m_saveFileName(std::move(fileName)),
      m_threads(count, nullptr),
      m_threadsStopSigns(count),
      m_threadCount(count) {
  for (uint32_t i = 0; i < m_threadCount; ++i) {
    m_threads[i] =
        new std::thread(worker, &m_storage, i, &m_threadsStopSigns[i]);
  }
}

HashArgumentFinder::~HashArgumentFinder() {
  for (uint32_t i = 0; i < m_threadCount; ++i) {
    m_threadsStopSigns[i] = true;
    m_threads[i]->join();
    delete m_threads[i];
  }
  m_storage.exportToFile(m_saveFileName);
}
