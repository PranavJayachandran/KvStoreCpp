#include <iostream>
#include "../include/kvstore/config/Config.h"
#include "../include/kvstore/engine/FileHandler.h"
int main(){
  std::cout<<"Staring";
  kvstore::config::LoadFromFile("../config.json");
  kvstore::engine::FileHandler file_handler;
  const std::string path = "/home/pranj/code/cpp/kvstore/temp.txt";
  std::string value = "world";
  file_handler.AppendToFile(path, value);  
}
