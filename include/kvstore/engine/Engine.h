#include "Memtable.h"
#include "MemtableManager.h"
#include "SST.h"
#include <memory>
namespace kvstore::engine {

template <typename K = std::string, typename V = std::string> class Engine {

  std::unique_ptr<SST<K, V>> sst;
  std::unique_ptr<MemtableManager<K, V>> memtable_manager;
  const int memtable_size = 100;

public:
  Engine() {
    kvstore::config::LoadFromFile("../config.json");
    sst = std::make_unique<SST<K, V>>();

    memtable_manager = std::make_unique<MemtableManager<K, V>>(
        memtable_size, [this](MemtableIterator<K, V> iterator) {
          sst->Flush(iterator);
          return true;
        });
  }
  ~Engine() {
    memtable_manager.reset();
    sst.reset();
  }

  void Add(const K &key, const V &value) { memtable_manager->Add(key, value); }

  void Delete(const K &key) { memtable_manager->Delete(key); }

  bool Get(const K &key, V &out_value) {
    if (memtable_manager->Get(key, out_value))
      return true;

    return sst->Get(key, out_value);
  }
};

} // namespace kvstore::engine
