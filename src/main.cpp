#include "../include/kvstore/config/Config.h"
#include "../include/kvstore/engine/Memtable.h"
#include "../include/kvstore/engine/SST.h"
#include <iostream>
#include <optional>
#include <thread>
#include <vector>

void check_for_multithread() {
  auto memtable = std::make_unique<kvstore::engine::Memtable<>>(4, "file_name");
  std::vector<std::thread> threads;
  for (int i = 0; i < 100; i++) {
    threads.emplace_back([&]() { memtable->Add("key", std::to_string(i)); });
  }

  for (auto &t : threads)
    t.join();

  std::string value;
  memtable->Get("key", value);
}
int main() {
  std::cout << "Staring";
  kvstore::config::LoadFromFile("../config.json");
  auto memtable = std::make_unique<kvstore::engine::Memtable<>>(4, "file_name");
  check_for_multithread();
  // kvstore::engine::SST<> sst;
  //
  // memtable->Add("key88", "value100");
  // memtable->Add("key4", "value5");
  // memtable->Add("key2", "value5");
  // memtable->Add("a123123", "value3");
  // if(memtable->ShouldFlush()){
  //   sst.Flush(memtable->GetMemtableITerator());
  //   memtable = std::make_unique<kvstore::engine::Memtable<>>(4);
  // }
}
