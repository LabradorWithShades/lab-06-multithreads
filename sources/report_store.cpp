// Copyright 2020 LabradorWithShades
// 70892548+LabradorWithShades@users.noreply.github.com

#include <fstream>
#include <iomanip>
#include <report_store.hpp>

ReportStorage::ReportStorage() : m_storage(json::array()) {}

void ReportStorage::addEntry(uint32_t timestamp, const std::string& hash,
                           const std::string& data) {
  json entry = R"(
  {
    "timestamp": 0,
    "hash": "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX0000",
    "data": "XXXXXXXXXXXXXXX"
  })"_json;

  entry["timestamp"] = timestamp;
  entry["hash"] = hash;
  entry["data"] = data;

  std::lock_guard lockGuard(m_mutex);
  m_storage.push_back(std::move(entry));
}

void ReportStorage::exportToFile(const std::string& filename) const {
  std::ofstream outFile;
  outFile.open(filename, std::ios::out);
  if (!outFile.is_open())
    throw std::runtime_error(
        "Report Store error: export to file failed: could not open file");
  std::lock_guard lockGuard(m_mutex);
  outFile << std::setw(4) << m_storage << std::endl;
  outFile.close();
}
