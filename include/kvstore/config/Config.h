#pragma once
#include <string>
#include <vector>

namespace kvstore::config {
struct Config {
  std::string sst_dir;
  size_t sst_key_block_size;
  size_t sst_value_block_size;
  std::vector<std::string> sst_level_folders;
  std::vector<int> sst_number_of_files_per_level, sst_size_of_file_per_level;
  int sst_level_count;
  std::string wal_dir;
};
void LoadFromFile(const std::string &filepath);
const Config &GetConfig();
} // namespace kvstore::config
