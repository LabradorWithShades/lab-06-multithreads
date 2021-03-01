// Copyright 2020 LabradorWithShades
// 70892548+LabradorWithShades@users.noreply.github.com

#ifndef INCLUDE_HASHARGUMENTFINDER_HPP_
#define INCLUDE_HASHARGUMENTFINDER_HPP_

#include <report_store.hpp>
#include <vector>
#include <thread>
#include <atomic>

class HashArgumentFinder {
 public:
  explicit HashArgumentFinder(
      std::string fileName,
      uint32_t count = std::thread::hardware_concurrency());
  ~HashArgumentFinder();

 private:
  ReportStorage m_storage;
  std::string m_saveFileName;
  std::vector<std::thread*> m_threads;
  std::vector<std::atomic<bool>> m_threadsStopSigns;
  uint32_t m_threadCount;
};

#endif  // INCLUDE_HASHARGUMENTFINDER_HPP_
