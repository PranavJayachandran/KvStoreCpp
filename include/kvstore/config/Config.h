#pragma once
#include <string>

namespace kvstore::config {
  struct Config{
    std::string sst_dir;
    size_t sst_key_block_size;
    size_t sst_value_block_size;
  };
  void LoadFromFile(const std::string &filepath);
  const Config& GetConfig();
}
