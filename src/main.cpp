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

  memtable->Add("key88", "value100");
  memtable->Add("key4", "value5");
  memtable->Add("key2", "value5");
  memtable->Add("a123123", "value3");
  if(memtable->ShouldFlush()){
    sst.Flush(memtable->GetMemtableITerator());
    memtable = std::make_unique<kvstore::engine::Memtable<>>(4);
  }
}
