#include <iostream>
#include "../include/kvstore/config/Config.h"
#include "../include/kvstore/engine/Memtable.h"
#include "../include/kvstore/engine/SST.h"
#include <ctime>

int main(){
  std::cout<<"Staring";
  kvstore::config::LoadFromFile("../config.json");
  auto memtable = std::make_unique<kvstore::engine::Memtable<>>(4);
  kvstore::engine::SST<> sst;

  memtable->Add("key7", "value100");
  memtable->Add("key1", "value5");
  memtable->Add("key0", "value5");
  memtable->Add("key9", "value3");
  std::time_t now = std::time(nullptr);
  sst.Flush(memtable->GetMemtableITerator());

  memtable = std::make_unique<kvstore::engine::Memtable<>>(4);
}
