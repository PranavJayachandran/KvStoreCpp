#pragma once
#include <string>
#include <vector>

namespace kvstore::config {
  struct Config{
    std::string sst_dir;
    size_t sst_key_block_size;
    size_t sst_value_block_size;
    std::vector<std::string> sst_level_folders;
  };
  void LoadFromFile(const std::string &filepath);
  const Config& GetConfig();
}
