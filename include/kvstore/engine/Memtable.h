#pragma once
#include "../config/Config.h"
#include "FileHandler.h"
#include "SkipList.h"
#include <filesystem>
#include <string>
namespace kvstore::engine {

template <typename K = std::string, typename V = std::string>
class MemtableIterator;

template <typename K = std::string, typename V = std::string> class Memtable {
private:
  size_t max_size_;
  SkipList<K, V> skiplist_;
  size_t current_size_ = 0;
  const std::string wal_dir = config::GetConfig().wal_dir;
  const std::string wal_file_name = "wal.txt";
  const int wal_key_block_size = config::GetConfig().sst_key_block_size;
  const int wal_value_block_size = config::GetConfig().sst_value_block_size;

  std::string FixSize(const K &key, int size) {
    std::string value;
    if (key.size() > size) {
      value = key.substr(0, size);
    } else {
      value = key + std::string(size - key.size(), '\0');
    }

    return value;
  }
  void WriteToWal(const K &key, const V &value, bool is_delete) {
    std::string data_to_write = FixSize(key, wal_key_block_size) +
                                FixSize(value, wal_value_block_size) +
                                (is_delete ? "1" : "0");
    FileHandler::AppendToFile(wal_dir + "/" + wal_file_name, data_to_write);
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
  void ReconstructUsingWal() {
    std::unique_ptr<std::ifstream> wal =
        FileHandler::GetPointerToFile(wal_dir + "/" + wal_file_name).stream;
    std::string buffer(wal_key_block_size + wal_value_block_size + 1, '\0');
    wal->read(&buffer[0], buffer.size());
    while (wal->gcount() == buffer.size()) {
      std::string key = GetKeyFromString(buffer),
                  value = GetValueFromString(buffer);
      std::cout << key << " " << value << "\n";
      bool is_deleted = IsTombstoneEntry(buffer);
      if (is_deleted) {
        skiplist_.Delete(key);
      } else {
        skiplist_.Add(key, value);
        current_size_ += sizeof(key) + sizeof(value);
      }
      wal->read(&buffer[0], buffer.size());
    }
  }

public:
  explicit Memtable(size_t size) : skiplist_(4) {
    std::filesystem::path wal_dir{"wal"};

    if (!std::filesystem::exists(wal_dir)) {
      std::filesystem::create_directories(wal_dir);
    }
    if (FileHandler::GetNumberofFiles(wal_dir) != 0) {
      ReconstructUsingWal();
    }
    max_size_ = size;
  }

  void Add(const K &key, const V &value) {
    skiplist_.Add(key, value);
    current_size_ += sizeof(key) + sizeof(value);
    WriteToWal(key, value, false);
  }

  bool Get(const K &key, V &out_value) {
    const V *val = skiplist_.Get(key);
    if (val) {
      out_value = *val;
      return true;
    }
    return false;
  }

  void Delete(const K &key) {
    skiplist_.Delete(key);
    std::string value = "";
    WriteToWal(key, value, true);
  }

  bool ShouldFlush() { return current_size_ < max_size_; }

  MemtableIterator<K, V> GetMemtableITerator() {
    return MemtableIterator<K, V>(skiplist_.GetSkipListIterator());
  }

  void Flush() {
    std::vector<std::string> wal_file = FileHandler::GetAllFileNames(wal_dir);
    // wal_file should only have 1 wal or no wal.
    std::string data = "";
    FileHandler::WriteToFile(wal_dir + "/" + wal_file_name, data);
  }
};

template <typename K, typename V> class MemtableIterator {
private:
  SkipListIterator<K, V> skip_list_iterator_;

public:
  explicit MemtableIterator(SkipListIterator<K, V> skip_list_iterator)
      : skip_list_iterator_(skip_list_iterator) {}
  bool HasNext() { return skip_list_iterator_.HasNext(); }

  std::tuple<K, V, bool> GetNext() { return skip_list_iterator_.GetNext(); }
};
} // namespace kvstore::engine
