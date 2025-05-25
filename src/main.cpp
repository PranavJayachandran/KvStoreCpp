#include <iostream>
#include <filesystem>
#include "../include/kvstore/config/Config.h"
#include "../include/kvstore/engine/FileHandler.h"
int main(){
  std::cout<<"Staring";
  kvstore::config::LoadFromFile("../config.json");
  kvstore::engine::FileHandler file_handler;

  file_handler.AppendToFile("/home/pranj/code/cpp/kvstore/temp.txt",  "wiorld");  
}
