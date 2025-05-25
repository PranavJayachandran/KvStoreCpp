// src/config/Config.cpp
#include "../include/kvstore/config/Config.h"
#include <fstream>
#include "../third_party/json.hpp" // optional, or use manual parsing

using json = nlohmann::json;

namespace kvstore::config{
  static Config globalConfig;
  void LoadFromFile(const std::string& filepath) {
    std::ifstream in(filepath);
    if (!in) throw std::runtime_error("Cannot open config file");

    json j;
    in >> j;

    std::string sst_dir;
    size_t sst_key_block_size;
    size_t sst_value_block_size;
    globalConfig = Config{
        j["sstDir"],
        j["sstKeyBlockSize"],
        j["sstValueBlockSize"],
    };
  }
  const Config& GetConfig(){
    return globalConfig;
  }
}

