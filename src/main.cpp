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
  std::string value;
  if(memtable->Get("key88",value)){
    std::cout<<value<<"\n";
  }
}
