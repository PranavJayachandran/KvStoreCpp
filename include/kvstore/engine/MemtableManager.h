#include "FileHandler.h"
#include "Memtable.h"
#include <algorithm>
#include <condition_variable>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace kvstore::engine {

template <typename K = std::string, typename V = std::string>
class MemtableManager {

private:
  std::unique_ptr<Memtable<>> active_memtable_;
  std::vector<std::unique_ptr<Memtable<>>> immutable_memtables;
  std::mutex swap_mutex, active_mutex, flush_mutex;
  std::thread flush_thread;
  std::condition_variable flush_cv;
  bool stop_flush = false;
  std::function<bool(MemtableIterator<K, V>)> flush_call_back;
  int memtable_size;
  const std::string wal_dir = config::GetConfig().wal_dir;
  const int wal_key_block_size = config::GetConfig().sst_key_block_size;
  const int wal_value_block_size = config::GetConfig().sst_value_block_size;

  std::condition_variable backpressure_cv;
  std::mutex backpressure_mutex;

  size_t max_immutable_memtables = 4;

  void SwapMemtables() {
    std::scoped_lock lock(active_mutex, swap_mutex);
    if (active_memtable_->ShouldFlush()) {
      immutable_memtables.push_back(std::move(active_memtable_));
      CreateNewMemtable();

      {
        std::lock_guard<std::mutex> lock(flush_mutex);
      }
      flush_cv.notify_one();
      backpressure_cv.notify_all();
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
    std::lock_guard<std::mutex> lock(swap_mutex);
    std::vector<std::filesystem::path> wal_files;
    std::cout << std::filesystem::exists(wal_dir);
    for (const auto &entry : std::filesystem::directory_iterator(wal_dir)) {
      if (std::filesystem::is_regular_file(entry.path())) {
        wal_files.push_back(entry.path());
      }
    }

    std::cout << wal_files.size();

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
      }
      // Last WAL file becomes active
      if (path == wal_files.back()) {
        active_memtable_ = std::move(memtable);
        auto p = active_memtable_->GetMemtableITerator();
        while (p.HasNext()) {
          std::tuple<K, V, bool> data = p.GetNext();
          std::cout << std::get<0>(data) << " " << std::get<1>(data) << " ";
        }
      } else {
        immutable_memtables.push_back(std::move(memtable));
      }
    }
  }
  void DeleteWal(const std::string &wal_file_name) {
    std::string file_path = wal_dir + '/' + wal_file_name;
    FileHandler::DeleteFile(file_path);
  }

  void FlushLoop() {
    while (true) {
      std::unique_ptr<Memtable<>> memtable_to_flush;
      {
        std::unique_lock<std::mutex> lock(flush_mutex);
        flush_cv.wait(
            lock, [&] { return stop_flush || !immutable_memtables.empty(); });

        if (stop_flush && immutable_memtables.empty())
          return;
      }
      {
        std::lock_guard<std::mutex> mgr_lock(swap_mutex);
        if (immutable_memtables.empty())
          continue;
        memtable_to_flush = std::move(immutable_memtables.front());
        immutable_memtables.erase(immutable_memtables.begin());
      }

      backpressure_cv.notify_all(); // <- REQUIRED
      if (memtable_to_flush == nullptr)
        continue;
      if (flush_call_back(memtable_to_flush->GetMemtableITerator()))
        DeleteWal(memtable_to_flush.get()->GetWalFileName());
    }
  }

  void WaitForBackpressure() {
    std::unique_lock<std::mutex> lock(swap_mutex);

    backpressure_cv.wait(lock, [&]() {
      return immutable_memtables.size() < max_immutable_memtables;
    });
  }

public:
  MemtableManager(size_t size,
                  std::function<bool(MemtableIterator<K, V>)> callback)
      : memtable_size(size), flush_call_back(callback) {
    ReconstructMemtableFromWal();

    if (!active_memtable_) {
      CreateNewMemtable();
    }

    flush_thread = std::thread(&MemtableManager<>::FlushLoop, this);
  }

  ~MemtableManager() {
    {
      std::lock_guard<std::mutex> lock(flush_mutex);
      stop_flush = true;
    }

    flush_cv.notify_all();

    if (flush_thread.joinable())
      flush_thread.join();
  }

  void Add(const K &key, const V &value) {

    WaitForBackpressure();
    {
      std::lock_guard<std::mutex> lock(active_mutex);
      active_memtable_->Add(key, value);
    }

    SwapMemtables();
  }
  bool Get(const K &key, V &out_value) {
    {
      std::scoped_lock lock(active_mutex);
      if (active_memtable_->Get(key, out_value)) {
        return true;
      }
    }

    {
      std::scoped_lock lock(swap_mutex);
      for (auto &imm : immutable_memtables) {
        if (imm->Get(key, out_value)) {
          return true;
        }
      }
    }
    return false;
  }
  void Delete(const K &key) {
    WaitForBackpressure();
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

    backpressure_cv.notify_all();
    return iterators;
  }
};
} // namespace kvstore::engine
