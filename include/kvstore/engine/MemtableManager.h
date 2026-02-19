#include "Memtable.h"
#include <algorithm>
#include <filesystem>
#include <memory>
#include <mutex>
#include <vector>

namespace kvstore::engine {

template <typename K = std::string, typename V = std::string>
class MemtableManager {

private:
  std::unique_ptr<Memtable<>> active_memtable_;
  std::vector<std::unique_ptr<Memtable<>>> immutable_memtables;
  std::mutex swap_mutex, active_mutex;
  int memtable_size;
  const std::string wal_dir = config::GetConfig().wal_dir;
  const int wal_key_block_size = config::GetConfig().sst_key_block_size;
  const int wal_value_block_size = config::GetConfig().sst_value_block_size;

  void SwapMemtables() {
    std::scoped_lock lock(active_mutex);
    if (active_memtable_->ShouldFlush()) {
      immutable_memtables.push_back(std::move(active_memtable_));
      CreateNewMemtable();
    }
  }

  void CreateNewMemtable() {
    active_memtable_ =
        std::make_unique<Memtable<>>(memtable_size, GetMemtableName());
  }

  std::string GetMemtableName() {
    auto now = std::chrono::system_clock::now();
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(
                      now.time_since_epoch())
                      .count();
    return "memtable_" + std::to_string(micros) + ".txt";
  }
  std::string GetKeyFromString(const std::string &buffer) {
    int i = 0;
    std::string key = "";
    while (i < wal_key_block_size) {
      if (buffer[i] == '\0')
        break;
      key += buffer[i];
      i++;
    }
    return key;
  }

  std::string GetValueFromString(const std::string &buffer) {
    int i = wal_key_block_size;
    std::string value = "";
    while (i < wal_key_block_size + wal_value_block_size) {
      if (buffer[i] == '\0')
        break;
      value += buffer[i];
      i++;
    }
    return value;
  }

  bool IsTombstoneEntry(const std::string &buffer) {
    return buffer[wal_key_block_size + wal_value_block_size] == '1';
  }
  void ReconstructMemtableFromWal() {
    std::vector<std::filesystem::path> wal_files;
    for (const auto &entry : std::filesystem::directory_iterator(wal_dir)) {
      if (std::filesystem::is_regular_file(entry.path())) {
        wal_files.push_back(entry.path());
      }
    }

    std::sort(wal_files.begin(), wal_files.end());

    for (const auto &path : wal_files) {
      auto memtable = std::make_unique<Memtable<K, V>>(
          memtable_size, path.filename().string());
      std::unique_ptr<std::ifstream> wal =
          FileHandler::GetPointerToFile(path).stream;
      std::string buffer(wal_key_block_size + wal_value_block_size + 1, '\0');
      while (wal->read(&buffer[0], buffer.size())) {
        std::string key = GetKeyFromString(buffer),
                    value = GetValueFromString(buffer);
        bool is_deleted = IsTombstoneEntry(buffer);
        memtable->InsertRecovered(key, value, is_deleted);
        wal->read(&buffer[0], buffer.size());
      }
      // Last WAL file becomes active
      if (path == wal_files.back()) {
        active_memtable_ = std::move(memtable);
      } else {
        immutable_memtables.push_back(std::move(memtable));
      }
    }
  }

public:
  MemtableManager(size_t size) : memtable_size(size) {
    ReconstructMemtableFromWal();

    if (!active_memtable_) {
      CreateNewMemtable();
    }
  }

  void Add(const K &key, const V &value) {
    {
      std::lock_guard<std::mutex> lock(active_mutex);
      active_memtable_->Add(key, value);
    }

    SwapMemtables();
  }
  bool Get(const K &key, V &out_value) {
    std::scoped_lock lock(active_mutex);
    if (active_memtable_->Get(key, out_value)) {
      return true;
    }

    {
      for (auto &imm : immutable_memtables) {
        if (imm->Get(key, out_value)) {
          return true;
        }
      }
    }
    return false;
  }
  void Delete(const K &key) {
    {
      std::lock_guard<std::mutex> lock(active_mutex);
      active_memtable_->Delete(key);
    }
    SwapMemtables();
  }

  std::vector<std::unique_ptr<Memtable<>>> GetImmuableMemtables() {
    std::lock_guard<std::mutex> lock(swap_mutex);
    std::vector<std::unique_ptr<Memtable<>>> iterators =
        std::move(immutable_memtables);
    return iterators;
  }
};
} // namespace kvstore::engine
