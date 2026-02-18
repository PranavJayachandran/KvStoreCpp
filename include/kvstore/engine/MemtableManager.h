#include "Memtable.h"
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

  void SwapMemtables() {
    std::scoped_lock lock(active_mutex, swap_mutex);
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

public:
  MemtableManager(size_t size) : memtable_size(size) { CreateNewMemtable(); }

  void Add(const K &key, const V &value) {
    {
      std::lock_guard<std::mutex> lock(active_mutex);
      active_memtable_->Add(key, value);
    }

    SwapMemtables();
  }
  bool Get(const K &key, V &out_value) {
    std::lock_guard<std::mutex> lock(active_mutex);
    if (active_memtable_->Get(key, out_value)) {
      return true;
    }

    {
      std::lock_guard<std::mutex> lock(swap_mutex);
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
