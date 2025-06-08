#include "../third_party/json.hpp"
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>

using json = nlohmann::json;

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

  static Config globalConfig;

  void LoadFromFile(const std::string& filepath) {
    std::ifstream in(filepath);
    if (!in) throw std::runtime_error("Cannot open config file");

    json j;
    in >> j;

    if (!j.contains("sstDir") || !j.contains("sstKeyBlockSize") ||
        !j.contains("sstValueBlockSize") || !j.contains("sstLevelFolders")) {
      throw std::runtime_error("Missing required config fields");
    }

    globalConfig = Config{
      j["sstDir"].get<std::string>(),
      j["sstKeyBlockSize"].get<size_t>(),
      j["sstValueBlockSize"].get<size_t>(),
      j["sstLevelFolders"].get<std::vector<std::string>>(),
      j["sstNumberOfFilesPerLevel"].get<std::vector<int>>(),
      j["sstSizeOfFilePerLevel"].get<std::vector<int>>(),
      j["sstLevelCount"].get<int>(),
      j["walDir"].get<std::string>()
    };
  }

  const Config& GetConfig() {
    return globalConfig;
  }
}
