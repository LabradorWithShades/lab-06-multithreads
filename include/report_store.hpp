// Copyright 2020 LabradorWithShades
// 70892548+LabradorWithShades@users.noreply.github.com

#ifndef INCLUDE_REPORT_STORE_HPP_
#define INCLUDE_REPORT_STORE_HPP_

#include <nlohmann/json.hpp>
#include <string>
#include <mutex>

using nlohmann::json;

class ReportStorage {
 public:
  ReportStorage();
  void addEntry(uint32_t timestamp, const std::string& hash,
                const std::string& data);
  void exportToFile(const std::string& filename) const;
 private:
  json m_storage;
  mutable std::mutex m_mutex{};
};

#endif  // INCLUDE_REPORT_STORE_HPP_
