// Copyright 2020 LabradorWithShades
// 70892548+LabradorWithShades@users.noreply.github.com

#ifndef INCLUDE_HASHARGUMENTFINDER_HPP_
#define INCLUDE_HASHARGUMENTFINDER_HPP_

#include <report_store.hpp>
#include <thread>
#include <vector>
#include <atomic>

class HashArgumentFinder {
 public:
  explicit HashArgumentFinder() noexcept;

  void startSearch(std::string fileName,
                   size_t count = std::thread::hardware_concurrency());
  void stopSearch();

  ~HashArgumentFinder();

 private:
  ReportStorage m_storage;
  std::string m_saveFileName;
  std::vector<std::shared_ptr<std::thread>> m_threads;
  std::vector<std::shared_ptr<std::atomic_bool>> m_threadsStopSigns;
  size_t m_threadCount;
  bool m_started;
};

#endif  // INCLUDE_HASHARGUMENTFINDER_HPP_
